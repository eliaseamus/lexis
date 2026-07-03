#include "library.hpp"

#include <QtSql/QtSql>

namespace lexis {

namespace {

QVariant nullParentId() {
  return QVariant();
}

}  // namespace

Library::Library(QObject* parent) : QObject(parent), _pronunciation(new Pronunciation(this)) {
  openDatabase("lexis.db");
  openLanguage(_settings.getCurrentLanguage());
  connect(_pronunciation, &Pronunciation::audioReady, this, &Library::updateAudio);
}

void Library::openDatabase(const QString& name) {
  QFileInfo info(name);
  QDir dir;
  dir.mkpath(info.dir().path());

  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName(info.filePath());
  if (!db.open()) {
    qWarning() << QString("Failed to open %1").arg(info.filePath());
    return;
  }

  QSqlQuery(db).exec("PRAGMA foreign_keys = ON");

  if (!SchemaMigration::ensureSchema(db)) {
    qWarning() << "Failed to initialize database schema";
  }
}

QVariant Library::parentIdVariant(int parentId) const {
  if (parentId == kRootParentId) {
    return nullParentId();
  }
  return parentId;
}

int Library::parentIdFromVariant(const QVariant& value) const {
  if (!value.isValid() || value.isNull()) {
    return kRootParentId;
  }
  return value.toInt();
}

void Library::ensureLanguage(const QString& language) {
  if (language.isEmpty()) {
    return;
  }
  QSqlQuery query;
  query.prepare("INSERT OR IGNORE INTO languages(code) VALUES (:code)");
  query.bindValue(":code", language);
  if (!query.exec()) {
    qWarning() << "Failed to register language:" << query.lastError();
  }
}

void Library::openLanguage(const QString& language) {
  if (language.isEmpty()) {
    _language.clear();
    _currentParentId = kRootParentId;
    clearSections();
    return;
  }

  ensureLanguage(language);
  _language = language;
  _currentParentId = kRootParentId;
  clearSections();
  populateSections();
}

void Library::openFolder(int parentId) {
  _currentParentId = parentId;
  clearSections();
  populateSections();
}

void Library::openRoot() {
  if (_language.isEmpty()) {
    clearSections();
    return;
  }
  _currentParentId = kRootParentId;
  clearSections();
  populateSections();
}

void Library::addItem(LibraryItem* item) {
  if (!item || _language.isEmpty()) {
    qWarning() << "no item was provided or language is not selected";
    return;
  }

  QSqlQuery query;
  query.prepare(
    "INSERT INTO items"
    "(language_code, parent_id, title, creation_time, modification_time, type, image, color)"
    "VALUES (:language_code, :parent_id, :title, :creation, :modification, :type, :image, :color)");

  const auto now = QDateTime::currentDateTime();
  item->setCreationTime(now);
  item->setModificationTime(now);
  query.bindValue(":language_code", _language);
  query.bindValue(":parent_id", parentIdVariant(_currentParentId));
  query.bindValue(":title", item->title());
  query.bindValue(":creation", item->creationTime().toString());
  query.bindValue(":modification", item->modificationTime().toString());
  query.bindValue(":type", std::to_underlying(item->type()));
  query.bindValue(":image", item->image());
  query.bindValue(":color", item->color().name());

  if (!query.exec()) {
    qWarning() << "Failed to insert item:" << query.lastError();
    return;
  }

  item->setID(query.lastInsertId().toInt());
  if (item->type() == LibrarySectionType::kWord) {
    _audioItem.id = item->id();
    requestAudio(item->title());
  }
  updateParentModificationTime(item->id());
  insertItem(std::move(*item));
}

void Library::updateItem(LibraryItem* item, LibrarySectionType oldType) {
  if (!item || _language.isEmpty()) {
    qWarning() << "no item was provided or language is not selected";
    return;
  }

  const auto oldTitle = getTitle(item->id());
  const bool wordTitleChanged =
    item->type() == LibrarySectionType::kWord && oldTitle != item->title();

  QSqlQuery query;
  if (wordTitleChanged) {
    _audioItem.id = item->id();
    requestAudio(item->title());
    query.prepare(
      "UPDATE items "
      "SET title = :title, modification_time = :modification, type = :type, image = :image, "
      "color = :color, audio = :audio "
      "WHERE id = :id AND language_code = :language_code");
    query.bindValue(":audio", QByteArray{});
  } else {
    query.prepare(
      "UPDATE items "
      "SET title = :title, modification_time = :modification, type = :type, image = :image, "
      "color = :color "
      "WHERE id = :id AND language_code = :language_code");
  }

  item->setModificationTime(QDateTime::currentDateTime());
  query.bindValue(":title", item->title());
  query.bindValue(":modification", item->modificationTime().toString());
  query.bindValue(":type", std::to_underlying(item->type()));
  query.bindValue(":image", item->image());
  query.bindValue(":color", item->color().name());
  query.bindValue(":id", item->id());
  query.bindValue(":language_code", _language);

  if (!query.exec()) {
    qWarning() << QString("Failed to update '%1' item:").arg(item->title()) << query.lastError();
    return;
  }

  updateParentModificationTime(item->id());

  if (oldType == item->type()) {
    getSection(item->type())->updateItem(std::move(*item));
  } else {
    auto* section = getSection(oldType);
    section->removeItem(item->id());
    if (section->isEmpty()) {
      _sections.removeOne(section);
    }
    insertItem(std::move(*item));
  }
}

void Library::moveItem(int id, int targetParentId) {
  if (_language.isEmpty()) {
    return;
  }

  QSqlQuery query;
  query.prepare(
    "UPDATE items SET parent_id = :parent_id, modification_time = :modification "
    "WHERE id = :id AND language_code = :language_code");
  query.bindValue(":parent_id", parentIdVariant(targetParentId));
  query.bindValue(":modification", QDateTime::currentDateTime().toString());
  query.bindValue(":id", id);
  query.bindValue(":language_code", _language);

  if (!query.exec()) {
    qWarning() << QString("Failed to move item %1:").arg(id) << query.lastError();
    return;
  }

  updateParentModificationTime(id);
  openFolder(_currentParentId);
}

void Library::deleteItem(int id, LibrarySectionType type) {
  if (_language.isEmpty()) {
    return;
  }

  updateParentModificationTime(id);
  QSqlQuery query;
  query.prepare("DELETE FROM items WHERE id = :id AND language_code = :language_code");
  query.bindValue(":id", id);
  query.bindValue(":language_code", _language);

  if (!query.exec()) {
    qWarning() << QString("Failed to delete item %1:").arg(id) << query.lastError();
    return;
  }

  auto* section = getSection(type);
  section->removeItem(id);
  if (section->isEmpty()) {
    _sections.removeOne(section);
  }
}

void Library::deleteLanguage(const QString& language) {
  if (language.isEmpty()) {
    return;
  }

  QSqlQuery query;
  query.prepare("DELETE FROM languages WHERE code = :code");
  query.bindValue(":code", language);
  if (!query.exec()) {
    qWarning() << QString("Failed to delete language %1:").arg(language) << query.lastError();
    return;
  }

  if (_language == language) {
    _language.clear();
    _currentParentId = kRootParentId;
    clearSections();
  }
}

QUrl Library::readAudio(int id) {
  QSqlQuery query;
  query.prepare("SELECT audio FROM items WHERE id = :id AND language_code = :language_code");
  query.bindValue(":id", id);
  query.bindValue(":language_code", _language);

  QByteArray audio;
  if (query.exec() && query.next()) {
    audio = query.value("audio").toByteArray();
  }

  return getSection(LibrarySectionType::kWord)->updateAudio(id, std::move(audio));
}

void Library::updateMeaning(int id, const QString& meaning) {
  QSqlQuery query;
  query.prepare("UPDATE items SET meaning = :meaning WHERE id = :id AND language_code = :language_code");
  query.bindValue(":meaning", meaning);
  query.bindValue(":id", id);
  query.bindValue(":language_code", _language);

  if (!query.exec()) {
    qWarning() << QString("Failed to update meaning for item %1:").arg(id) << query.lastError();
    return;
  }

  getSection(LibrarySectionType::kWord)->updateMeaning(id, meaning);
}

TreeModel* Library::getStructure() {
  static TreeModel* prevStructure = nullptr;
  auto headerData = QVariantList{"title", "parentId"};
  auto header = std::make_unique<TreeItem>(headerData);
  auto rootData = QVariantList{tr("Start page"), kRootParentId};
  auto root = std::make_unique<TreeItem>(rootData, header.get());

  addChildItems(kRootParentId, root.get());
  header->appendChild(std::move(root));

  if (prevStructure) {
    prevStructure->deleteLater();
  }
  auto* model = new TreeModel(this);
  model->setRoot(std::move(header));
  prevStructure = model;
  return model;
}

void Library::clearSections() {
  for (auto* section : _sections) {
    section->deleteLater();
  }
  _sections.clear();
}

LibrarySection* Library::getSection(LibrarySectionType type) {
  auto pos = std::find_if(_sections.begin(), _sections.end(), [type](LibrarySection* section) {
    return section->type() == type;
  });

  if (pos != _sections.end()) {
    return *pos;
  }

  auto* section = new LibrarySection(type, this);
  _sections.push_back(section);
  return section;
}

void Library::populateSections() {
  if (_language.isEmpty()) {
    return;
  }

  QSqlQuery query;
  if (_currentParentId == kRootParentId) {
    query.prepare(
      "SELECT id, title, creation_time, modification_time, type, image, color, meaning "
      "FROM items WHERE language_code = :language_code AND parent_id IS NULL");
  } else {
    query.prepare(
      "SELECT id, title, creation_time, modification_time, type, image, color, meaning "
      "FROM items WHERE language_code = :language_code AND parent_id = :parent_id");
    query.bindValue(":parent_id", _currentParentId);
  }
  query.bindValue(":language_code", _language);

  if (!query.exec()) {
    qWarning() << "Failed to populate sections:" << query.lastError();
    return;
  }

  while (query.next()) {
    LibraryItem item;
    item.setID(query.value("id").toInt());
    item.setTitle(query.value("title").toString());
    item.setCreationTime(QDateTime::fromString(query.value("creation_time").toString()));
    item.setModificationTime(QDateTime::fromString(query.value("modification_time").toString()));
    item.setType(_typeManager.librarySectionType(query.value("type").toInt()));
    item.setImage(query.value("image").toByteArray());
    item.setColor(query.value("color").toString());
    item.setMeaning(query.value("meaning").toString());
    insertItem(std::move(item));
  }
}

void Library::insertItem(LibraryItem&& item) {
  getSection(item.type())->addItem(std::move(item));
}

QString Library::getTitle(int id) const {
  QSqlQuery query;
  query.prepare("SELECT title FROM items WHERE id = :id AND language_code = :language_code");
  query.bindValue(":id", id);
  query.bindValue(":language_code", _language);

  if (query.exec() && query.next()) {
    return query.value("title").toString();
  }
  return {};
}

void Library::requestAudio(const QString& title) {
  _pronunciation->get(title);
}

void Library::updateParentModificationTime(int id) {
  QSqlQuery query;
  query.prepare("SELECT parent_id FROM items WHERE id = :id AND language_code = :language_code");
  query.bindValue(":id", id);
  query.bindValue(":language_code", _language);

  if (!query.exec() || !query.next()) {
    return;
  }

  const auto parentId = parentIdFromVariant(query.value("parent_id"));
  if (parentId == kRootParentId) {
    return;
  }

  QSqlQuery updateQuery;
  updateQuery.prepare(
    "UPDATE items SET modification_time = :modification "
    "WHERE id = :id AND language_code = :language_code");
  updateQuery.bindValue(":modification", QDateTime::currentDateTime().toString());
  updateQuery.bindValue(":id", parentId);
  updateQuery.bindValue(":language_code", _language);

  if (!updateQuery.exec()) {
    qWarning() << QString("Failed to update parent item %1:").arg(parentId) << updateQuery.lastError();
    return;
  }

  updateParentModificationTime(parentId);
}

void Library::addChildItems(int parentId, TreeItem* parent) {
  QSqlQuery query;
  if (parentId == kRootParentId) {
    query.prepare(
      "SELECT id, title, type FROM items "
      "WHERE language_code = :language_code AND parent_id IS NULL");
  } else {
    query.prepare(
      "SELECT id, title, type FROM items "
      "WHERE language_code = :language_code AND parent_id = :parent_id");
    query.bindValue(":parent_id", parentId);
  }
  query.bindValue(":language_code", _language);

  if (!query.exec()) {
    qWarning() << "Failed to build move tree:" << query.lastError();
    return;
  }

  while (query.next()) {
    const auto id = query.value("id").toInt();
    const auto title = query.value("title").toString();
    const auto type = _typeManager.librarySectionType(query.value("type").toInt());
    if (type == LibrarySectionType::kWord) {
      continue;
    }

    auto itemData = QVariantList{title, id};
    auto item = std::make_unique<TreeItem>(itemData, parent);
    addChildItems(id, item.get());
    parent->appendChild(std::move(item));
  }
}

void Library::updateAudio(QByteArray audio) {
  QSqlQuery query;
  query.prepare("UPDATE items SET audio = :audio WHERE id = :id AND language_code = :language_code");
  query.bindValue(":audio", audio);
  query.bindValue(":id", _audioItem.id);
  query.bindValue(":language_code", _language);

  if (!query.exec()) {
    qWarning() << QString("Failed to update audio for item %1:").arg(_audioItem.id)
               << query.lastError();
  }
}

}  // namespace lexis

#include "library.hpp"

#include <QtSql/QtSql>

namespace lexis {

Library::Library(QObject* parent) :
  QObject(parent),
  _pronunciation(new Pronunciation(this))
{
  openDatabase("lexis.db");
  openTable(_settings.getCurrentLanguage());
  connect(_pronunciation, &Pronunciation::audioReady, this, &Library::updateAudio);
}

void Library::openDatabase(const QString& name) {
  QFileInfo info(name);
  QDir dir;
  dir.mkpath(info.dir().path());
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName(info.filePath());
  if (!db.open()) {
    qDebug() << QString("Failed to open %1").arg(info.filePath()).toStdString();
  }
}

void Library::addItem(LibraryItem* item) {
  if (!item) {
    qWarning() << "no item was provided";
    return;
  }

  QSqlQuery query;
  query.prepare(
    QString(
      "INSERT INTO %1"
      "(title, creation_time, modification_time, type, image, color)"
      "VALUES (:title, :creation, :modification, :type, :image, :color)"
    ).arg(_table)
  );

  auto now = QDateTime::currentDateTime();
  item->setCreationTime(now);
  item->setModificationTime(now);
  query.bindValue(":title", item->title());
  query.bindValue(":creation", item->creationTime().toString());
  query.bindValue(":modification", item->modificationTime().toString());
  query.bindValue(":type", std::to_underlying(item->type()));
  query.bindValue(":image", item->image());
  query.bindValue(":color", item->color().name());

  if (!query.exec()) {
    qWarning() << QString("Failed to insert item into '%1' table:").arg(_table)
               << query.lastError();
    return;
  }

  item->setID(getItemID(item->title(), _table));
  if (item->type() == LibrarySectionType::kWord) {
    _audioItem.table = _table;
    _audioItem.id = item->id();
    requestAudio(item->title());
  } else {
    createChildTable(item->id());
  }
  updateParentModificationTime(_table, item->id());
  insertItem(std::move(*item));
}

void Library::updateItem(LibraryItem* item, LibrarySectionType oldType) {
  if (!item) {
    qWarning() << "no item was provided";
    return;
  }

  auto oldTitle = getTitle(item->id(), _table);
  QSqlQuery query;
  query.prepare(
    QString(
      "UPDATE %1 "
      "SET title = :title, modification_time = :modification, type = :type, "
          "image = :image, color = :color "
      "WHERE id = \"%2\""
    ).arg(_table, QString::number(item->id()))
  );

  item->setModificationTime(QDateTime::currentDateTime());
  query.bindValue(":title", item->title());
  query.bindValue(":modification", item->modificationTime().toString());
  query.bindValue(":type", std::to_underlying(item->type()));
  query.bindValue(":image", item->image());
  query.bindValue(":color", item->color().name());

  if (!query.exec()) {
    qWarning() << QString("Failed to update '%1' item in '%2' table:")
                  .arg(item->title(), _table) << query.lastError();
    return;
  }

  if (item->type() == LibrarySectionType::kWord && oldTitle != item->title()) {
    _audioItem.table = _table;
    _audioItem.id = item->id();
    requestAudio(item->title());
  }
  updateParentModificationTime(_table, item->id());

  if (oldType == item->type()) {
    auto* section = getSection(item->type());
    section->updateItem(std::move(*item));
  } else {
    auto* section = getSection(oldType);
    section->removeItem(item->id());
    if (section->isEmpty()) {
      _sections.removeOne(section);
    }
    insertItem(std::move(*item));
  }

}

void Library::moveItem(int id, const QString& sourceTable, const QString& targetTable) {
  auto item = readItem(id, sourceTable);
  if (!removeItem(id, item.type(), sourceTable)) {
    return;
  }
  if (!insertItem(item, targetTable)) {
    return;
  }

  if (item.type() == LibrarySectionType::kWord) {
    return;
  }

  const auto oldName = QString("%1_%2").arg(sourceTable, QString::number(id));
  renameTableRecursively(oldName, targetTable, getItemID(item.title(), targetTable));
}

void Library::deleteItem(int id, LibrarySectionType type) {
  updateParentModificationTime(_table, id);
  QSqlQuery query;
  query.prepare(QString("DELETE FROM %1 WHERE id = \"%2\"").arg(_table, QString::number(id)));
  if (!query.exec()) {
    qWarning() << QString("Failed to delete item of id = '%1' from '%2' table:")
                  .arg(QString::number(id), _table) << query.lastError();
    return;
  }
  if (type != LibrarySectionType::kWord) {
    dropTableRecursively(QString("%1_%2").arg(_table, QString::number(id)));
  }
  auto* section = getSection(type);
  section->removeItem(id);
  if (section->isEmpty()) {
    _sections.removeOne(section);
  }
}

void Library::createTable() {
  QSqlQuery query;
  auto createTableQuery = QString(
    "CREATE TABLE IF NOT EXISTS %1                        \
    (id                INTEGER PRIMARY KEY AUTOINCREMENT, \
     creation_time     TEXT,                              \
     modification_time TEXT,                              \
     title             TEXT,                              \
     type              INTEGER,                           \
     image             BLOB,                              \
     color             TEXT,                              \
     audio             BLOB,                              \
     meaning           TEXT)"
  ).arg(_table);
  if (!query.exec(createTableQuery)) {
    qDebug() << QString("failed to create %1 table").arg(_table) << query.lastError();
  }
}

void Library::createChildTable(int parentID) {
  QSqlQuery query;
  auto parentIDString = QString::number(parentID);
  auto childTable = QString("%1_%2").arg(_table, parentIDString);
  auto createTableQuery = QString(
    "CREATE TABLE IF NOT EXISTS %1                        \
    (id                INTEGER PRIMARY KEY AUTOINCREMENT, \
     parent_table      TEXT DEFAULT %2,                   \
     parent_id         INTEGER DEFAULT %3,                \
     creation_time     TEXT,                              \
     modification_time TEXT,                              \
     title             TEXT,                              \
     type              INTEGER,                           \
     image             BLOB,                              \
     color             TEXT,                              \
     audio             BLOB,                              \
     meaning           TEXT)"
  ).arg(childTable, _table, parentIDString);
  if (!query.exec(createTableQuery)) {
    qDebug() << QString("failed to create %1 table").arg(childTable) << query.lastError();
  }
}

void Library::openTable(const QString& name) {
  if (name.isEmpty()) {
    return;
  }
  _table = name;
  clearSections();
  createTable();
  populateSections();
}

void Library::openChildTable(int parentID) {
  clearSections();
  _table = QString("%1_%2").arg(_table, QString::number(parentID));
  populateSections();
}

void Library::dropTableRecursively(const QString& root) {
  for (const auto& table : getTablesList()) {
    if (table.startsWith(root)) {
      dropTable(table);
    }
  }
}

void Library::readAudio(int id) {
  QSqlQuery query(
    QString(
      "SELECT audio FROM %1 WHERE id = \"%2\""
    ).arg(_table, QString::number(id))
  );

  QByteArray audio;
  while (query.next()) {
    audio = query.value("audio").toByteArray();
  }

  auto* section = getSection(LibrarySectionType::kWord);
  section->updateAudio(id, std::move(audio));
}

void Library::updateMeaning(int id, const QString& meaning) {
  QSqlQuery query;
  query.prepare(
    QString(
      "UPDATE %1 "
      "SET meaning = :meaning "
      "WHERE id = \"%2\""
    ).arg(_table, QString::number(id))
  );

  query.bindValue(":meaning", meaning);

  if (!query.exec()) {
    qWarning() << QString("Failed to update '%1' item in '%2' table:")
                  .arg(QString::number(id), _table) << query.lastError();
    return;
  }

  auto* section = getSection(LibrarySectionType::kWord);
  section->updateMeaning(id, meaning);
}

TreeModel* Library::getStructure() {
  static TreeModel* prevStructure = nullptr;
  auto headerData = QVariantList() << "title" << "table";
  auto header = std::make_unique<TreeItem>(headerData);
  auto language = _settings.getCurrentLanguage();
  auto rootData = QVariantList() << tr("Start page") << language;
  auto root = std::make_unique<TreeItem>(rootData, header.get());

  addChildItems(language, root.get());
  header->appendChild(std::move(root));

  if (prevStructure) {
    prevStructure->deleteLater();
  }
  auto* model = new TreeModel(this);
  model->setRoot(std::move(header));
  prevStructure = model;
  return model;
}

LibraryItem Library::readItem(int id, const QString& table) {
  QSqlQuery query(
    QString(
      "SELECT id, title, creation_time, modification_time, type, image, color, "
      "meaning, audio FROM %1 WHERE id = %2"
    ).arg(table, QString::number(id))
  );

  LibraryItem item;
  if (query.next()) {
    item.setID(query.value("id").toInt());
    item.setTitle(query.value("title").toString());
    item.setCreationTime(QDateTime::fromString(query.value("creation_time").toString()));
    item.setModificationTime(QDateTime::fromString(query.value("modification_time").toString()));
    item.setType(_typeManager.librarySectionType(query.value("type").toInt()));
    item.setImage(query.value("image").toByteArray());
    item.setColor(query.value("color").toString());
    item.setAudio(query.value("audio").toByteArray());
    item.setMeaning(query.value("meaning").toString());
  } else {
    qDebug() << QString("Failed to read item of id = %1 from %2 table")
                .arg(QString::number(id), table);
  }
  return item;
}

bool Library::removeItem(int id, LibrarySectionType type, const QString& table) {
  updateParentModificationTime(table, id);
  QSqlQuery query;
  query.prepare(QString("DELETE FROM %1 WHERE id = %2")
                .arg(table, QString::number(id)));
  if (!query.exec()) {
    qWarning() << QString("Failed to delete item of id = '%1' from '%2' table:")
                  .arg(QString::number(id), table) << query.lastError();
    return false;
  }
  if (table == _table) {
    auto* section = getSection(type);
    section->removeItem(id);
    if (section->isEmpty()) {
      _sections.removeOne(section);
    }
  }
  return true;
}

bool Library::insertItem(LibraryItem& item, const QString& table) {
  QSqlQuery query;
  query.prepare(
    QString(
      "INSERT INTO %1"
      "(title, creation_time, modification_time, type, image, color, audio, meaning)"
      "VALUES (:title, :creation, :modification, :type, :image, :color, :audio, :meaning)"
    ).arg(table)
  );

  item.setModificationTime(QDateTime::currentDateTime());
  query.bindValue(":title", item.title());
  query.bindValue(":creation", item.creationTime().toString());
  query.bindValue(":modification", item.modificationTime().toString());
  query.bindValue(":type", std::to_underlying(item.type()));
  query.bindValue(":image", item.image());
  query.bindValue(":color", item.color().name());
  query.bindValue(":audio", item.audio());
  query.bindValue(":meaning", item.meaning());

  if (!query.exec()) {
    qWarning() << QString("Failed to insert item into '%1' table:").arg(table)
               << query.lastError();
    return false;
  }
  return true;
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
  QSqlQuery query(
    QString(
      "SELECT id, title, creation_time, modification_time, type, image, color, meaning FROM %1"
    ).arg(_table)
  );

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
  auto* section = getSection(item.type());
  section->addItem(std::move(item));
}

QStringList Library::getTablesList() const {
  QSqlQuery query("SELECT name FROM sqlite_master WHERE type = 'table'");
  QStringList list;
  while (query.next()) {
    list.append(query.value("name").toString());
  }
  return list;
}

int Library::getItemID(const QString& title, const QString& table) const {
  QSqlQuery query(
    QString(
      "SELECT id FROM %1 WHERE title = \"%2\""
    ).arg(table, title)
  );
  int id = -1;
  while (query.next()) {
    id = query.value("id").toInt();
  }
  return id;
}

QString Library::getTitle(int id, const QString& table) const {
  QSqlQuery query(
    QString(
      "SELECT title FROM %1 WHERE id = \"%2\""
    ).arg(table, QString::number(id))
  );
  QString title;
  while (query.next()) {
    title = query.value("title").toString();
  }
  return title;
}

void Library::dropTable(const QString& name) {
  if (name.isEmpty()) {
    return;
  }
  QSqlQuery query;
  auto deleteTableQuery = QString("DROP TABLE IF EXISTS %1").arg(name);
  if (!query.exec(deleteTableQuery)) {
    qDebug() << "failed to delete table" << name << query.lastError();
    return;
  }
  if (name == _table) {
    _sections.clear();
    _table.clear();
  }
}

void Library::requestAudio(const QString& title) {
  _pronunciation->get(title);
}

void Library::updateParentModificationTime(const QString& table, int id) {
  QSqlQuery query(
    QString(
      "SELECT parent_table, parent_id FROM %1 WHERE id = \"%2\""
    ).arg(table, QString::number(id))
  );
  while (query.next()) {
    auto parentTable = query.value("parent_table").toString();
    auto parentID = query.value("parent_id").toInt();
    QSqlQuery updateQuery;
    updateQuery.prepare(
      QString(
        "UPDATE %1 "
        "SET modification_time = :modification "
        "WHERE id = \"%2\""
      ).arg(parentTable, QString::number(parentID))
    );
    updateQuery.bindValue(":modification", QDateTime::currentDateTime().toString());
    if (!updateQuery.exec()) {
      qWarning() << QString("Failed to update '%1' item in '%2' table:")
                    .arg(QString::number(parentID), parentTable) << query.lastError();
      return;
    }
    updateParentModificationTime(parentTable, parentID);
  }
}

void Library::renameTableRecursively(const QString& oldName,
                                     const QString& parentTable, int parentId) {
  const auto newName = QString("%1_%2").arg(parentTable, QString::number(parentId));
  if (!renameTable(oldName, newName)) {
    return;
  }
  if (!updateParentInfo(newName, parentTable, parentId)) {
    return;
  }

  QSqlQuery query(QString("SELECT id, type FROM %1").arg(newName));
  while (query.next()) {
    auto id = query.value("id").toInt();
    auto type = _typeManager.librarySectionType(query.value("type").toInt());
    if (type != LibrarySectionType::kWord) {
      const auto childOldName = QString("%1_%2").arg(oldName, QString::number(id));
      renameTableRecursively(childOldName, newName, id);
    }
  }
}

bool Library::renameTable(const QString& oldName, const QString& newName) {
  QSqlQuery query;
  query.prepare(
    QString(
      "ALTER TABLE %1 RENAME TO %2"
    ).arg(oldName, newName)
  );

  if (!query.exec()) {
    qWarning() << QString("Failed to rename %1 table to %2:").arg(oldName, newName)
               << query.lastError();
    return false;
  }
  return true;
}

bool Library::updateParentInfo(const QString& table, const QString& parentTable, int parentId) {
  QSqlQuery query;
  query.prepare(
    QString(
      "UPDATE %1 "
      "SET parent_table = :parent_table, parent_id = :parent_id"
    ).arg(table)
  );

  query.bindValue(":parent_table", parentTable);
  query.bindValue(":parent_id", QString::number(parentId));

  if (!query.exec()) {
    qWarning() << QString("Failed to update parent info in '%1' table:").arg(table)
               << query.lastError();
    return false;
  }
  return true;
}

void Library::addChildItems(const QString& table, TreeItem* parent) {
  QSqlQuery query(QString("SELECT id, title, type FROM %1").arg(table));
  while (query.next()) {
    auto id = query.value("id").toInt();
    auto title = query.value("title").toString();
    auto type = _typeManager.librarySectionType(query.value("type").toInt());
    if (type != LibrarySectionType::kWord) {
      auto childTable = QString("%1_%2").arg(table, QString::number(id));
      auto itemData = QVariantList() << title << childTable;
      auto item = std::make_unique<TreeItem>(itemData, parent);
      addChildItems(childTable, item.get());
      parent->appendChild(std::move(item));
    }
  }
}

void Library::updateAudio(QByteArray audio) {
  QSqlQuery query;
  query.prepare(
    QString(
      "UPDATE %1 SET audio = :audio WHERE id = \"%2\""
    ).arg(_audioItem.table, QString::number(_audioItem.id))
  );

  query.bindValue(":audio", audio);

  if (!query.exec()) {
    qWarning() << QString("Failed to update '%1' item in '%2' table:")
                  .arg(QString::number(_audioItem.id), _audioItem.table) << query.lastError();
    return;
  }
}

}

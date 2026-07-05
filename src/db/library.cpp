#include "library.hpp"

#include "library_archive.hpp"
#include "frequency_lookup.hpp"
#include "library_search.hpp"
#include "library_statistics.hpp"
#include "utils.hpp"

#include <QtSql/QtSql>

namespace lexis {

namespace {

QVariant nullParentId() {
  return QVariant();
}

void applyFrequencyToItem(LibraryItem* item, const QString& languageCode) {
  if (!item) {
    return;
  }

  if (item->type() != LibrarySectionType::kWord) {
    item->setFrequencyRank(-1);
    item->setFrequencyTier({});
    return;
  }

  if (!FrequencyLookup::isOpen()) {
    return;
  }

  const auto result = FrequencyLookup::lookup(languageCode, item->title());
  if (result.found) {
    item->setFrequencyRank(result.rank);
    item->setFrequencyTier(result.tier);
  } else {
    item->setFrequencyRank(-1);
    item->setFrequencyTier({});
  }
}

void bindFrequencyValues(QSqlQuery& query, const LibraryItem* item) {
  if (item->frequencyRank() > 0 && !item->frequencyTier().isEmpty()) {
    query.bindValue(":frequency_rank", item->frequencyRank());
    query.bindValue(":frequency_tier", item->frequencyTier());
  } else {
    query.bindValue(":frequency_rank", QVariant());
    query.bindValue(":frequency_tier", QVariant());
  }
}

}  // namespace

Library::Library(QObject* parent) : QObject(parent), _pronunciation(new Pronunciation(this)) {
  openDatabase("lexis.db");
  openLanguage(_settings.getCurrentLanguage());
  connect(_pronunciation, &Pronunciation::audioReady, this, &Library::updateAudio);
  connect(_pronunciation, &Pronunciation::audioFailed, this, [this]() {
    const auto itemId = _pendingAudioItemId > 0 ? _pendingAudioItemId : _audioItem.id;
    finishAudioFetch(itemId, {});
  });
}

void Library::closeDatabaseConnection() {
  if (!QSqlDatabase::contains(QSqlDatabase::defaultConnection)) {
    return;
  }
  const auto connectionName = QSqlDatabase::defaultConnection;
  QSqlDatabase db = QSqlDatabase::database(connectionName);
  db.close();
  QSqlDatabase::removeDatabase(connectionName);
}

void Library::openDatabase(const QString& name) {
  QFileInfo info(name);
  QDir dir;
  dir.mkpath(info.dir().path());
  _databasePath = info.absoluteFilePath();

  closeDatabaseConnection();

  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName(_databasePath);
  if (!db.open()) {
    qWarning() << QString("Failed to open %1").arg(_databasePath);
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
  backfillMissingFrequencies();
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

  applyFrequencyToItem(item, _language);

  QSqlQuery query;
  query.prepare(
    "INSERT INTO items"
    "(language_code, parent_id, title, creation_time, modification_time, type, image, color, "
    "frequency_rank, frequency_tier)"
    "VALUES (:language_code, :parent_id, :title, :creation, :modification, :type, :image, :color, "
    ":frequency_rank, :frequency_tier)");

  const auto now = QDateTime::currentDateTime();
  item->setCreationTime(now);
  item->setModificationTime(now);
  query.bindValue(":language_code", _language);
  query.bindValue(":parent_id", parentIdVariant(_currentParentId));
  query.bindValue(":title", item->title());
  query.bindValue(":creation", formatDateTimeForDb(item->creationTime()));
  query.bindValue(":modification", formatDateTimeForDb(item->modificationTime()));
  query.bindValue(":type", std::to_underlying(item->type()));
  query.bindValue(":image", item->image());
  query.bindValue(":color", item->color().name());
  bindFrequencyValues(query, item);

  if (!query.exec()) {
    qWarning() << "Failed to insert item:" << query.lastError();
    return;
  }

  item->setID(query.lastInsertId().toInt());
  if (item->type() == LibrarySectionType::kWord) {
    _audioItem.id = item->id();
    requestAudioForItem(item->id(), item->title());
  }
  updateParentModificationTime(item->id());
  invalidateImageCache(item->id());
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
  const bool frequencyNeedsRefresh =
    item->type() == LibrarySectionType::kWord || oldType == LibrarySectionType::kWord;
  if (frequencyNeedsRefresh) {
    applyFrequencyToItem(item, _language);
  }

  QSqlQuery query;
  if (wordTitleChanged) {
    _audioItem.id = item->id();
    invalidateAudioCache(item->id());
    requestAudioForItem(item->id(), item->title());
    query.prepare(
      "UPDATE items "
      "SET title = :title, modification_time = :modification, type = :type, image = :image, "
      "color = :color, audio = :audio, frequency_rank = :frequency_rank, "
      "frequency_tier = :frequency_tier "
      "WHERE id = :id AND language_code = :language_code");
    query.bindValue(":audio", QByteArray{});
  } else {
    query.prepare(
      "UPDATE items "
      "SET title = :title, modification_time = :modification, type = :type, image = :image, "
      "color = :color, frequency_rank = :frequency_rank, frequency_tier = :frequency_tier "
      "WHERE id = :id AND language_code = :language_code");
  }

  item->setModificationTime(QDateTime::currentDateTime());
  query.bindValue(":title", item->title());
  query.bindValue(":modification", formatDateTimeForDb(item->modificationTime()));
  query.bindValue(":type", std::to_underlying(item->type()));
  query.bindValue(":image", item->image());
  query.bindValue(":color", item->color().name());
  bindFrequencyValues(query, item);
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
  invalidateImageCache(item->id());
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
  query.bindValue(":modification", formatDateTimeForDb(QDateTime::currentDateTime()));
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
  invalidateImageCache(id);
  invalidateAudioCache(id);
  for (int i = _audioRequestQueue.size() - 1; i >= 0; --i) {
    if (_audioRequestQueue[i].itemId == id) {
      _audioRequestQueue.removeAt(i);
    }
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
  if (_language.isEmpty() || id <= 0) {
    return {};
  }

  if (_audioUrlCache.contains(id)) {
    return _audioUrlCache.value(id);
  }

  _audioItem.id = id;

  QSqlQuery query;
  query.prepare("SELECT audio FROM items WHERE id = :id AND language_code = :language_code");
  query.bindValue(":id", id);
  query.bindValue(":language_code", _language);

  QByteArray audio;
  if (query.exec() && query.next()) {
    audio = query.value("audio").toByteArray();
  }

  if (audio.isEmpty()) {
    const auto title = getTitle(id);
    if (!title.isEmpty()) {
      requestAudioForItem(id, title);
    }
    return {};
  }

  const auto url = cacheAudioBlob(id, audio);
  if (url.isEmpty()) {
    QSqlQuery clearQuery;
    clearQuery.prepare(
      "UPDATE items SET audio = :audio WHERE id = :id AND language_code = :language_code");
    clearQuery.bindValue(":audio", QByteArray{});
    clearQuery.bindValue(":id", id);
    clearQuery.bindValue(":language_code", _language);
    clearQuery.exec();
    getSection(LibrarySectionType::kWord)->updateAudio(id, QByteArray{});

    const auto title = getTitle(id);
    if (!title.isEmpty()) {
      requestAudioForItem(id, title);
    }
    return {};
  }

  getSection(LibrarySectionType::kWord)->updateAudio(id, QByteArray(audio));
  return url;
}

void Library::refreshAudio(int id) {
  if (_language.isEmpty() || id <= 0) {
    return;
  }

  _audioItem.id = id;
  const auto title = getTitle(id);
  if (title.isEmpty()) {
    return;
  }

  invalidateAudioCache(id);
  for (int i = _audioRequestQueue.size() - 1; i >= 0; --i) {
    if (_audioRequestQueue[i].itemId == id) {
      _audioRequestQueue.removeAt(i);
    }
  }

  QSqlQuery query;
  query.prepare("UPDATE items SET audio = :audio WHERE id = :id AND language_code = :language_code");
  query.bindValue(":audio", QByteArray{});
  query.bindValue(":id", id);
  query.bindValue(":language_code", _language);
  if (!query.exec()) {
    qWarning() << QString("Failed to clear audio for item %1:").arg(id) << query.lastError();
  }

  getSection(LibrarySectionType::kWord)->updateAudio(id, QByteArray{});
  requestAudioForItem(id, title);
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

void Library::updateCachedTranslation(int id, const QString& translation) {
  if (_language.isEmpty() || id <= 0) {
    return;
  }

  QSqlQuery query;
  query.prepare(
    "UPDATE items SET cached_translation = :translation "
    "WHERE id = :id AND language_code = :language_code");
  query.bindValue(":translation", translation);
  query.bindValue(":id", id);
  query.bindValue(":language_code", _language);

  if (!query.exec()) {
    qWarning() << QString("Failed to update cached translation for item %1:").arg(id)
               << query.lastError();
  }
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
  invalidateImageCache();
  invalidateAudioCache();
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

void Library::backfillMissingFrequencies() {
  if (_language.isEmpty() || !FrequencyLookup::isOpen()) {
    return;
  }

  QSqlQuery query;
  query.prepare(
    "SELECT id, title FROM items "
    "WHERE language_code = :language_code AND type = :word_type");
  query.bindValue(":language_code", _language);
  query.bindValue(":word_type", std::to_underlying(LibrarySectionType::kWord));

  if (!query.exec()) {
    qWarning() << "Failed to query words for frequency backfill:" << query.lastError();
    return;
  }

  while (query.next()) {
    LibraryItem item;
    item.setID(query.value("id").toInt());
    item.setTitle(query.value("title").toString());
    item.setType(LibrarySectionType::kWord);
    applyFrequencyToItem(&item, _language);
    if (item.frequencyRank() > 0) {
      persistFrequency(item.id(), item.frequencyRank(), item.frequencyTier());
    }
  }
}

void Library::populateSections() {
  if (_language.isEmpty()) {
    return;
  }

  QSqlQuery query;
  if (_currentParentId == kRootParentId) {
    query.prepare(
      "SELECT id, title, creation_time, modification_time, type, image, color, meaning, "
      "frequency_rank, frequency_tier "
      "FROM items WHERE language_code = :language_code AND parent_id IS NULL");
  } else {
    query.prepare(
      "SELECT id, title, creation_time, modification_time, type, image, color, meaning, "
      "frequency_rank, frequency_tier "
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

    const auto storedRank = query.value("frequency_rank");
    if (storedRank.isValid() && !storedRank.isNull()) {
      item.setFrequencyRank(storedRank.toInt());
      item.setFrequencyTier(query.value("frequency_tier").toString());
    }

    if (item.type() == LibrarySectionType::kWord &&
        (item.frequencyRank() <= 0 || item.frequencyTier().isEmpty())) {
      applyFrequencyToItem(&item, _language);
      if (item.frequencyRank() > 0) {
        persistFrequency(item.id(), item.frequencyRank(), item.frequencyTier());
      }
    }

    insertItem(std::move(item));
  }
}

void Library::persistFrequency(int id, int rank, const QString& tier) {
  if (_language.isEmpty() || id <= 0) {
    return;
  }

  QSqlQuery query;
  query.prepare(
    "UPDATE items SET frequency_rank = :frequency_rank, frequency_tier = :frequency_tier "
    "WHERE id = :id AND language_code = :language_code");
  if (rank > 0 && !tier.isEmpty()) {
    query.bindValue(":frequency_rank", rank);
    query.bindValue(":frequency_tier", tier);
  } else {
    query.bindValue(":frequency_rank", QVariant());
    query.bindValue(":frequency_tier", QVariant());
  }
  query.bindValue(":id", id);
  query.bindValue(":language_code", _language);
  if (!query.exec()) {
    qWarning() << "Failed to persist frequency for item" << id << query.lastError();
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

void Library::requestAudioForItem(int id, const QString& title) {
  if (id <= 0 || title.isEmpty()) {
    return;
  }

  for (const auto& request : _audioRequestQueue) {
    if (request.itemId == id) {
      return;
    }
  }
  if (_audioFetchInProgress && _pendingAudioItemId == id) {
    return;
  }

  _audioRequestQueue.append(PendingAudioRequest{id, title});
  processAudioQueue();
}

void Library::processAudioQueue() {
  if (_audioFetchInProgress || _audioRequestQueue.isEmpty()) {
    return;
  }

  const auto request = _audioRequestQueue.constFirst();
  _audioFetchInProgress = true;
  _pendingAudioItemId = request.itemId;
  _audioItem.id = request.itemId;
  _pronunciation->get(request.title);
}

void Library::finishAudioFetch(int itemId, const QByteArray& audio) {
  _audioFetchInProgress = false;
  _pendingAudioItemId = -1;
  for (int i = 0; i < _audioRequestQueue.size(); ++i) {
    if (_audioRequestQueue[i].itemId == itemId) {
      _audioRequestQueue.removeAt(i);
      break;
    }
  }

  if (itemId <= 0) {
    processAudioQueue();
    return;
  }

  if (audio.isEmpty()) {
    emit audioReady(itemId, QUrl{});
    processAudioQueue();
    return;
  }

  QSqlQuery query;
  query.prepare("UPDATE items SET audio = :audio WHERE id = :id AND language_code = :language_code");
  query.bindValue(":audio", audio);
  query.bindValue(":id", itemId);
  query.bindValue(":language_code", _language);

  if (!query.exec()) {
    qWarning() << QString("Failed to update audio for item %1:").arg(itemId) << query.lastError();
    emit audioReady(itemId, QUrl{});
    processAudioQueue();
    return;
  }

  const auto url = cacheAudioBlob(itemId, audio);
  getSection(LibrarySectionType::kWord)->updateAudio(itemId, QByteArray(audio));
  emit audioReady(itemId, url);
  processAudioQueue();
}

void Library::updateAudio(QByteArray audio) {
  const auto itemId = _pendingAudioItemId > 0 ? _pendingAudioItemId : _audioItem.id;
  finishAudioFetch(itemId, std::move(audio));
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
  updateQuery.bindValue(":modification", formatDateTimeForDb(QDateTime::currentDateTime()));
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

bool Library::exportLanguage(const QString& language, const QUrl& fileUrl) {
  if (language.isEmpty()) {
    return false;
  }
  QSqlDatabase db = QSqlDatabase::database();
  return LibraryArchive::exportLanguage(db, language, fileUrl.toLocalFile());
}

bool Library::importLanguage(const QUrl& fileUrl) {
  QSqlDatabase db = QSqlDatabase::database();
  QString language;
  if (!LibraryArchive::importLanguage(db, fileUrl.toLocalFile(), &language)) {
    return false;
  }

  if (_language == language) {
    openLanguage(language);
  }
  emit languageImported(language);
  return true;
}

bool Library::backupDatabase(const QUrl& fileUrl) {
  return LibraryArchive::backupDatabase(_databasePath, fileUrl.toLocalFile());
}

bool Library::restoreDatabase(const QUrl& fileUrl) {
  closeDatabaseConnection();
  if (!LibraryArchive::restoreDatabase(_databasePath, fileUrl.toLocalFile())) {
    openDatabase(_databasePath);
    return false;
  }

  openDatabase(_databasePath);
  openLanguage(_settings.getCurrentLanguage());
  emit databaseRestored();
  return true;
}

QStringList Library::registeredLanguages() const {
  QStringList languages;
  QSqlQuery query("SELECT code FROM languages ORDER BY code");
  if (!query.exec()) {
    qWarning() << "Failed to read registered languages:" << query.lastError();
    return languages;
  }
  while (query.next()) {
    languages.append(query.value(0).toString());
  }
  return languages;
}

QVariantList Library::search(const QString& query) const {
  if (_language.isEmpty()) {
    return {};
  }
  return LibrarySearch::search(QSqlDatabase::database(), _language, query, _typeManager);
}

QVariantList Library::findByTitle(const QString& title, int excludeItemId) const {
  if (_language.isEmpty()) {
    return {};
  }
  return LibrarySearch::findByTitle(QSqlDatabase::database(), _language, title, _typeManager,
                                    excludeItemId);
}

QVariantMap Library::languageStatistics() const {
  if (_language.isEmpty()) {
    return {};
  }
  return LibraryStatistics::languageStats(QSqlDatabase::database(), _language, _typeManager);
}

QVariantMap Library::itemStatistics(int itemId) const {
  if (_language.isEmpty()) {
    return {};
  }
  return LibraryStatistics::itemStats(QSqlDatabase::database(), _language, itemId, _typeManager);
}

QVariantList Library::wordsInScope(int scopeRootId) const {
  if (_language.isEmpty()) {
    return {};
  }
  return LibraryStatistics::scopedWords(QSqlDatabase::database(), _language, scopeRootId);
}

QVariantList Library::ancestorPath(int itemId) const {
  if (_language.isEmpty() || itemId <= 0) {
    return {};
  }
  const auto index = LibrarySearch::loadItemIndex(QSqlDatabase::database(), _language);
  return LibrarySearch::ancestorPath(index, itemId);
}

LibraryItem* Library::getItem(int id) {
  if (_language.isEmpty() || id <= 0) {
    return nullptr;
  }

  QSqlQuery query;
  query.prepare(
    "SELECT id, title, creation_time, modification_time, type, image, color, meaning, "
    "frequency_rank, frequency_tier "
    "FROM items WHERE id = :id AND language_code = :language_code");
  query.bindValue(":id", id);
  query.bindValue(":language_code", _language);

  if (!query.exec() || !query.next()) {
    qWarning() << "Failed to load item" << id << query.lastError();
    return nullptr;
  }

  auto* item = new LibraryItem(this);
  item->setID(query.value("id").toInt());
  item->setTitle(query.value("title").toString());
  item->setCreationTime(QDateTime::fromString(query.value("creation_time").toString()));
  item->setModificationTime(QDateTime::fromString(query.value("modification_time").toString()));
  item->setType(_typeManager.librarySectionType(query.value("type").toInt()));
  item->setImage(query.value("image").toByteArray());
  item->setColor(query.value("color").toString());
  item->setMeaning(query.value("meaning").toString());
  const auto storedRank = query.value("frequency_rank");
  if (storedRank.isValid() && !storedRank.isNull()) {
    item->setFrequencyRank(storedRank.toInt());
    item->setFrequencyTier(query.value("frequency_tier").toString());
  }
  if (item->type() == LibrarySectionType::kWord &&
      (item->frequencyRank() <= 0 || item->frequencyTier().isEmpty())) {
    applyFrequencyToItem(item, _language);
    if (item->frequencyRank() > 0) {
      persistFrequency(item->id(), item->frequencyRank(), item->frequencyTier());
    }
  }
  return item;
}

void Library::invalidateImageCache(int id) {
  if (id < 0) {
    for (auto* file : _imageFiles) {
      file->deleteLater();
    }
    _imageFiles.clear();
    _imageUrlCache.clear();
    return;
  }

  if (auto* file = _imageFiles.take(id)) {
    file->deleteLater();
  }
  _imageUrlCache.remove(id);
}

void Library::invalidateAudioCache(int id) {
  if (id < 0) {
    for (auto* file : _audioFiles) {
      file->deleteLater();
    }
    _audioFiles.clear();
    _audioUrlCache.clear();
    return;
  }

  if (auto* file = _audioFiles.take(id)) {
    file->deleteLater();
  }
  _audioUrlCache.remove(id);
}

QUrl Library::cacheAudioBlob(int id, const QByteArray& compressedAudio) {
  if (id <= 0 || compressedAudio.isEmpty()) {
    return {};
  }

  invalidateAudioCache(id);

  auto* file = new QTemporaryFile(this);
  file->setFileTemplate(temporaryFileTemplate());
  file->setAutoRemove(true);
  if (!writeCompressedBlob(*file, compressedAudio)) {
    file->deleteLater();
    return {};
  }

  const auto url = QUrl::fromLocalFile(file->fileName());
  _audioFiles.insert(id, file);
  _audioUrlCache.insert(id, url);
  return url;
}

QUrl Library::itemImageUrl(int id) {
  if (id <= 0 || _language.isEmpty()) {
    return {};
  }

  if (_imageUrlCache.contains(id)) {
    return _imageUrlCache[id];
  }

  QSqlQuery query;
  query.prepare("SELECT image FROM items WHERE id = :id AND language_code = :language_code");
  query.bindValue(":id", id);
  query.bindValue(":language_code", _language);

  if (!query.exec() || !query.next()) {
    return {};
  }

  const auto image = query.value("image").toByteArray();
  if (image.isEmpty()) {
    return {};
  }

  auto* file = new QTemporaryFile(this);
  file->setFileTemplate(temporaryFileTemplate());
  file->setAutoRemove(true);
  if (!writeCompressedBlob(*file, image)) {
    file->deleteLater();
    return {};
  }

  const auto url = QUrl::fromLocalFile(file->fileName());
  _imageFiles.insert(id, file);
  _imageUrlCache.insert(id, url);
  return url;
}

}  // namespace lexis

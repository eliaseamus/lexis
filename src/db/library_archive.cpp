#include "library_archive.hpp"

#include "schema_migration.hpp"

#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QSqlError>
#include <QSqlQuery>

namespace lexis {

namespace {

constexpr int kWordType = 0;

QString localPath(const QString& filePath) {
  return QUrl::fromUserInput(filePath).toLocalFile();
}

bool exec(QSqlQuery& query, const char* context) {
  if (query.exec()) {
    return true;
  }
  qWarning() << context << query.lastError();
  return false;
}

}  // namespace

QJsonObject LibraryArchive::buildManifest(const QString& language, int itemCount, int wordCount) {
  QJsonObject manifest;
  manifest["format_version"] = kArchiveFormatVersion;
  manifest["schema_version"] = kSchemaVersion;
  manifest["language"] = language;
  manifest["exported_at"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
  manifest["item_count"] = itemCount;
  manifest["word_count"] = wordCount;
  return manifest;
}

QList<LibraryArchive::ArchiveItem> LibraryArchive::readItems(QSqlDatabase& db,
                                                             const QString& language) {
  QList<ArchiveItem> items;
  QSqlQuery query(db);
  query.prepare(
    "SELECT id, parent_id, title, type, creation_time, modification_time, color, meaning, image, "
    "audio, frequency_rank, frequency_tier "
    "FROM items WHERE language_code = :language_code ORDER BY id");
  query.bindValue(":language_code", language);

  if (!exec(query, "read items for export:")) {
    return items;
  }

  while (query.next()) {
    ArchiveItem item;
    item.id = query.value("id").toInt();
    if (!query.value("parent_id").isNull()) {
      item.hasParent = true;
      item.parentId = query.value("parent_id").toInt();
    }
    item.title = query.value("title").toString();
    item.type = query.value("type").toInt();
    item.creationTime = query.value("creation_time").toString();
    item.modificationTime = query.value("modification_time").toString();
    item.color = query.value("color").toString();
    item.meaning = query.value("meaning").toString();
    item.image = query.value("image").toByteArray();
    item.audio = query.value("audio").toByteArray();
    const auto storedRank = query.value("frequency_rank");
    if (storedRank.isValid() && !storedRank.isNull()) {
      item.frequencyRank = storedRank.toInt();
      item.frequencyTier = query.value("frequency_tier").toString();
    }
    items.append(item);
  }

  return items;
}

QJsonObject LibraryArchive::archiveItemToJson(const ArchiveItem& item) {
  QJsonObject object;
  object["id"] = item.id;
  object["parent_id"] = item.hasParent ? QJsonValue(item.parentId) : QJsonValue(QJsonValue::Null);
  object["title"] = item.title;
  object["type"] = item.type;
  object["creation_time"] = item.creationTime;
  object["modification_time"] = item.modificationTime;
  object["color"] = item.color;
  object["meaning"] = item.meaning;
  if (!item.image.isEmpty()) {
    object["image"] = QString::fromLatin1(item.image.toBase64());
  }
  if (!item.audio.isEmpty()) {
    object["audio"] = QString::fromLatin1(item.audio.toBase64());
  }
  if (item.frequencyRank > 0 && !item.frequencyTier.isEmpty()) {
    object["frequency_rank"] = item.frequencyRank;
    object["frequency_tier"] = item.frequencyTier;
  }
  return object;
}

LibraryArchive::ArchiveItem LibraryArchive::archiveItemFromJson(const QJsonObject& object) {
  ArchiveItem item;
  item.id = object["id"].toInt();
  if (!object["parent_id"].isNull()) {
    item.hasParent = true;
    item.parentId = object["parent_id"].toInt();
  }
  item.title = object["title"].toString();
  item.type = object["type"].toInt();
  item.creationTime = object["creation_time"].toString();
  item.modificationTime = object["modification_time"].toString();
  item.color = object["color"].toString();
  item.meaning = object["meaning"].toString();
  if (object.contains("image")) {
    item.image = QByteArray::fromBase64(object["image"].toString().toLatin1());
  }
  if (object.contains("audio")) {
    item.audio = QByteArray::fromBase64(object["audio"].toString().toLatin1());
  }
  if (object.contains("frequency_rank")) {
    item.frequencyRank = object["frequency_rank"].toInt();
    item.frequencyTier = object["frequency_tier"].toString();
  }
  return item;
}

bool LibraryArchive::writeArchiveFile(const QString& filePath, const QJsonObject& root) {
  QSaveFile file(localPath(filePath));
  if (!file.open(QIODevice::WriteOnly)) {
    qWarning() << "Failed to open archive for writing:" << file.errorString();
    return false;
  }

  const QJsonDocument document(root);
  if (file.write(document.toJson(QJsonDocument::Indented)) < 0) {
    qWarning() << "Failed to write archive:" << file.errorString();
    return false;
  }

  if (!file.commit()) {
    qWarning() << "Failed to commit archive:" << file.errorString();
    return false;
  }
  return true;
}

bool LibraryArchive::readArchiveFile(const QString& filePath, QJsonObject* root) {
  QFile file(localPath(filePath));
  if (!file.open(QIODevice::ReadOnly)) {
    qWarning() << "Failed to open archive for reading:" << file.errorString();
    return false;
  }

  QJsonParseError error;
  const auto document = QJsonDocument::fromJson(file.readAll(), &error);
  if (error.error != QJsonParseError::NoError || !document.isObject()) {
    qWarning() << "Invalid archive JSON:" << error.errorString();
    return false;
  }

  *root = document.object();
  return true;
}

QList<LibraryArchive::ArchiveItem> LibraryArchive::sortItemsForImport(
  const QList<ArchiveItem>& items) {
  QHash<int, ArchiveItem> byId;
  for (const auto& item : items) {
    byId.insert(item.id, item);
  }

  QList<ArchiveItem> sorted;
  QSet<int> inserted;
  while (sorted.size() < items.size()) {
    const auto previousSize = sorted.size();
    for (const auto& item : items) {
      if (inserted.contains(item.id)) {
        continue;
      }
      if (!item.hasParent || inserted.contains(item.parentId)) {
        sorted.append(item);
        inserted.insert(item.id);
      }
    }
    if (sorted.size() == previousSize) {
      qWarning() << "Archive items contain a cycle or missing parent";
      return {};
    }
  }
  return sorted;
}

bool LibraryArchive::exportLanguage(QSqlDatabase& db, const QString& language,
                                    const QString& filePath) {
  if (language.isEmpty()) {
    qWarning() << "Cannot export an empty language code";
    return false;
  }

  const auto items = readItems(db, language);
  int wordCount = 0;
  QJsonArray itemArray;
  for (const auto& item : items) {
    if (item.type == kWordType) {
      ++wordCount;
    }
    itemArray.append(archiveItemToJson(item));
  }

  QJsonObject root;
  root["manifest"] = buildManifest(language, items.size(), wordCount);
  root["items"] = itemArray;
  return writeArchiveFile(filePath, root);
}

bool LibraryArchive::importLanguage(QSqlDatabase& db, const QString& filePath,
                                    QString* importedLanguage) {
  QJsonObject root;
  if (!readArchiveFile(filePath, &root)) {
    return false;
  }

  const auto manifest = root["manifest"].toObject();
  if (manifest["format_version"].toInt() != kArchiveFormatVersion) {
    qWarning() << "Unsupported archive format version";
    return false;
  }
  if (manifest["schema_version"].toInt() > kSchemaVersion) {
    qWarning() << "Archive requires a newer database schema";
    return false;
  }

  const auto language = manifest["language"].toString();
  if (language.isEmpty()) {
    qWarning() << "Archive is missing language code";
    return false;
  }

  QList<ArchiveItem> items;
  for (const auto& value : root["items"].toArray()) {
    items.append(archiveItemFromJson(value.toObject()));
  }

  const auto sortedItems = sortItemsForImport(items);
  if (sortedItems.isEmpty() && !items.isEmpty()) {
    return false;
  }

  if (!db.transaction()) {
    qWarning() << "Failed to start import transaction:" << db.lastError();
    return false;
  }

  QSqlQuery deleteItems(db);
  deleteItems.prepare("DELETE FROM items WHERE language_code = :language_code");
  deleteItems.bindValue(":language_code", language);
  if (!exec(deleteItems, "clear language before import:")) {
    db.rollback();
    return false;
  }

  QSqlQuery registerLanguage(db);
  registerLanguage.prepare("INSERT OR IGNORE INTO languages(code) VALUES (:code)");
  registerLanguage.bindValue(":code", language);
  if (!exec(registerLanguage, "register imported language:")) {
    db.rollback();
    return false;
  }

  QHash<int, int> idMap;
  for (const auto& item : sortedItems) {
    QSqlQuery insert(db);
    insert.prepare(
      "INSERT INTO items"
      "(language_code, parent_id, title, creation_time, modification_time, type, image, color, "
      "audio, meaning, frequency_rank, frequency_tier)"
      "VALUES (:language_code, :parent_id, :title, :creation_time, :modification_time, :type, "
      ":image, :color, :audio, :meaning, :frequency_rank, :frequency_tier)");

    insert.bindValue(":language_code", language);
    if (item.hasParent) {
      insert.bindValue(":parent_id", idMap.value(item.parentId));
    } else {
      insert.bindValue(":parent_id", QVariant());
    }
    insert.bindValue(":title", item.title);
    insert.bindValue(":creation_time", item.creationTime);
    insert.bindValue(":modification_time", item.modificationTime);
    insert.bindValue(":type", item.type);
    insert.bindValue(":image", item.image);
    insert.bindValue(":color", item.color);
    insert.bindValue(":audio", item.audio);
    insert.bindValue(":meaning", item.meaning);
    if (item.frequencyRank > 0 && !item.frequencyTier.isEmpty()) {
      insert.bindValue(":frequency_rank", item.frequencyRank);
      insert.bindValue(":frequency_tier", item.frequencyTier);
    } else {
      insert.bindValue(":frequency_rank", QVariant());
      insert.bindValue(":frequency_tier", QVariant());
    }

    if (!exec(insert, "insert imported item:")) {
      db.rollback();
      return false;
    }

    idMap.insert(item.id, insert.lastInsertId().toInt());
  }

  if (!db.commit()) {
    qWarning() << "Failed to commit import:" << db.lastError();
    return false;
  }

  if (importedLanguage != nullptr) {
    *importedLanguage = language;
  }
  return true;
}

bool LibraryArchive::backupDatabase(const QString& databasePath, const QString& filePath) {
  const auto source = localPath(databasePath);
  const auto destination = localPath(filePath);
  if (!QFile::exists(source)) {
    qWarning() << "Database file does not exist:" << source;
    return false;
  }

  if (QFile::exists(destination) && !QFile::remove(destination)) {
    qWarning() << "Failed to replace existing backup file:" << destination;
    return false;
  }

  if (!QFile::copy(source, destination)) {
    qWarning() << "Failed to copy database to backup";
    return false;
  }
  return true;
}

bool LibraryArchive::restoreDatabase(const QString& databasePath, const QString& filePath) {
  const auto source = localPath(filePath);
  const auto destination = localPath(databasePath);
  if (!QFile::exists(source)) {
    qWarning() << "Backup file does not exist:" << source;
    return false;
  }

  const auto tempPath = destination + ".restore";
  if (QFile::exists(tempPath) && !QFile::remove(tempPath)) {
    qWarning() << "Failed to prepare temporary restore file:" << tempPath;
    return false;
  }

  if (!QFile::copy(source, tempPath)) {
    qWarning() << "Failed to copy backup into temporary restore file";
    return false;
  }

  if (QFile::exists(destination) && !QFile::remove(destination)) {
    qWarning() << "Failed to replace existing database file:" << destination;
    QFile::remove(tempPath);
    return false;
  }

  if (!QFile::rename(tempPath, destination)) {
    qWarning() << "Failed to finalize database restore";
    QFile::remove(tempPath);
    return false;
  }
  return true;
}

}  // namespace lexis

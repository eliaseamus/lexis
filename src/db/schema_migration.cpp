#include "schema_migration.hpp"

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace lexis {

namespace {

bool exec(QSqlQuery& query, const char* context) {
  if (query.exec()) {
    return true;
  }
  qWarning() << context << query.lastError();
  return false;
}

QString childLegacyTableName(const QString& parentTable, int itemId) {
  return QString("%1_%2").arg(parentTable, QString::number(itemId));
}

bool tableExists(QSqlDatabase& db, const QString& name) {
  QSqlQuery query(db);
  query.prepare("SELECT 1 FROM sqlite_master WHERE type = 'table' AND name = :name");
  query.bindValue(":name", name);
  if (!query.exec()) {
    return false;
  }
  return query.next();
}

QStringList allUserTables(QSqlDatabase& db) {
  QSqlQuery query("SELECT name FROM sqlite_master WHERE type = 'table'", db);
  QStringList tables;
  while (query.next()) {
    tables.append(query.value(0).toString());
  }
  return tables;
}

bool execSql(QSqlDatabase& db, const char* sql) {
  QSqlQuery query(db);
  if (query.exec(sql)) {
    return true;
  }
  qWarning() << sql << query.lastError();
  return false;
}

}  // namespace

bool SchemaMigration::createSchemaV1(QSqlDatabase& db) {
  const char* statements[] = {
    "CREATE TABLE IF NOT EXISTS schema_version (version INTEGER NOT NULL)",
    "CREATE TABLE IF NOT EXISTS languages (code TEXT PRIMARY KEY)",
    "CREATE TABLE IF NOT EXISTS items ("
    "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  language_code TEXT NOT NULL REFERENCES languages(code) ON DELETE CASCADE,"
    "  parent_id INTEGER REFERENCES items(id) ON DELETE CASCADE,"
    "  title TEXT NOT NULL,"
    "  type INTEGER NOT NULL,"
    "  creation_time TEXT NOT NULL,"
    "  modification_time TEXT NOT NULL,"
    "  color TEXT,"
    "  meaning TEXT,"
    "  cached_translation TEXT,"
    "  frequency_rank INTEGER,"
    "  frequency_tier TEXT,"
    "  image BLOB,"
    "  audio BLOB"
    ")",
    "CREATE INDEX IF NOT EXISTS idx_items_language ON items(language_code)",
    "CREATE INDEX IF NOT EXISTS idx_items_parent ON items(parent_id)",
    "CREATE INDEX IF NOT EXISTS idx_items_title ON items(title COLLATE NOCASE)",
  };

  for (const auto* sql : statements) {
    if (!execSql(db, sql)) {
      return false;
    }
  }

  if (currentVersion(db) == 0) {
    return setVersion(db, 1);
  }
  return true;
}

bool SchemaMigration::migrateToV2(QSqlDatabase& db) {
  QSqlQuery query(db);
  query.prepare("PRAGMA table_info(items)");
  if (!query.exec()) {
    qWarning() << "read items schema:" << query.lastError();
    return false;
  }

  while (query.next()) {
    if (query.value("name").toString() == QStringLiteral("cached_translation")) {
      return true;
    }
  }

  return execSql(db, "ALTER TABLE items ADD COLUMN cached_translation TEXT");
}

bool SchemaMigration::migrateToV3(QSqlDatabase& db) {
  QSqlQuery query(db);
  query.prepare("PRAGMA table_info(items)");
  if (!query.exec()) {
    qWarning() << "read items schema:" << query.lastError();
    return false;
  }

  bool hasFrequencyRank = false;
  bool hasFrequencyTier = false;
  while (query.next()) {
    const auto column = query.value("name").toString();
    if (column == QStringLiteral("frequency_rank")) {
      hasFrequencyRank = true;
    } else if (column == QStringLiteral("frequency_tier")) {
      hasFrequencyTier = true;
    }
  }

  if (!hasFrequencyRank && !execSql(db, "ALTER TABLE items ADD COLUMN frequency_rank INTEGER")) {
    return false;
  }
  if (!hasFrequencyTier && !execSql(db, "ALTER TABLE items ADD COLUMN frequency_tier TEXT")) {
    return false;
  }
  return true;
}

bool SchemaMigration::upgradeSchema(QSqlDatabase& db) {
  if (!migrateToV2(db)) {
    return false;
  }
  if (!migrateToV3(db)) {
    return false;
  }
  if (currentVersion(db) < kSchemaVersion) {
    return setVersion(db, kSchemaVersion);
  }
  return true;
}

int SchemaMigration::currentVersion(QSqlDatabase& db) {
  if (!tableExists(db, "schema_version")) {
    return 0;
  }
  QSqlQuery query("SELECT version FROM schema_version LIMIT 1", db);
  if (!query.exec() || !query.next()) {
    return 0;
  }
  return query.value(0).toInt();
}

bool SchemaMigration::setVersion(QSqlDatabase& db, int version) {
  QSqlQuery query(db);
  if (!query.exec("DELETE FROM schema_version")) {
    qWarning() << "clear schema_version:" << query.lastError();
    return false;
  }
  query.prepare("INSERT INTO schema_version(version) VALUES (:version)");
  query.bindValue(":version", version);
  return exec(query, "set schema_version:");
}

bool SchemaMigration::isLegacyTableName(const QString& name) {
  if (name == "schema_version" || name == "languages" || name == "items" ||
      name == "sqlite_sequence") {
    return false;
  }
  if (name.startsWith("sqlite_")) {
    return false;
  }
  const auto parts = name.split('_');
  if (parts.isEmpty() || parts.front().size() != 2) {
    return false;
  }
  for (const auto& part : parts) {
    if (part.isEmpty()) {
      return false;
    }
    if (part.size() == 2 && part == parts.front()) {
      continue;
    }
    for (const auto ch : part) {
      if (!ch.isDigit()) {
        return false;
      }
    }
  }
  return true;
}

QStringList SchemaMigration::legacyRootTables(QSqlDatabase& db) {
  QStringList roots;
  for (const auto& name : allUserTables(db)) {
    if (!name.contains('_') && name.size() == 2 && isLegacyTableName(name)) {
      roots.append(name);
    }
  }
  return roots;
}

bool SchemaMigration::migrateLegacyTable(QSqlDatabase& db, const QString& tableName,
                                         const QString& languageCode,
                                         std::optional<int> parentId) {
  QSqlQuery select(db);
  select.prepare(QString("SELECT id, title, creation_time, modification_time, type, image, color, "
                         "meaning, audio FROM %1")
                   .arg(tableName));
  if (!exec(select, "read legacy table:")) {
    return false;
  }

  while (select.next()) {
    const auto oldId = select.value("id").toInt();
    const auto type = select.value("type").toInt();

    QSqlQuery insert(db);
    insert.prepare(
      "INSERT INTO items"
      "(language_code, parent_id, title, creation_time, modification_time, type, image, color, "
      "meaning, audio)"
      "VALUES (:language_code, :parent_id, :title, :creation_time, :modification_time, :type, "
      ":image, :color, :meaning, :audio)");
    insert.bindValue(":language_code", languageCode);
    if (parentId.has_value()) {
      insert.bindValue(":parent_id", *parentId);
    } else {
      insert.bindValue(":parent_id", QVariant());
    }
    insert.bindValue(":title", select.value("title"));
    insert.bindValue(":creation_time", select.value("creation_time"));
    insert.bindValue(":modification_time", select.value("modification_time"));
    insert.bindValue(":type", type);
    insert.bindValue(":image", select.value("image"));
    insert.bindValue(":color", select.value("color"));
    insert.bindValue(":meaning", select.value("meaning"));
    insert.bindValue(":audio", select.value("audio"));

    if (!exec(insert, "insert migrated item:")) {
      return false;
    }

    const auto newId = insert.lastInsertId().toInt();
    if (type != 0) {
      const auto childTable = childLegacyTableName(tableName, oldId);
      if (tableExists(db, childTable)) {
        if (!migrateLegacyTable(db, childTable, languageCode, newId)) {
          return false;
        }
      }
    }
  }

  return true;
}

void SchemaMigration::dropLegacyTables(QSqlDatabase& db) {
  for (const auto& name : allUserTables(db)) {
    if (isLegacyTableName(name)) {
      QSqlQuery query(db);
      query.exec(QString("DROP TABLE IF EXISTS %1").arg(name));
    }
  }
}

bool SchemaMigration::migrateFromLegacy(QSqlDatabase& db) {
  if (!db.transaction()) {
    qWarning() << "Failed to start migration transaction:" << db.lastError();
    return false;
  }

  if (!createSchemaV1(db)) {
    db.rollback();
    return false;
  }

  for (const auto& language : legacyRootTables(db)) {
    QSqlQuery languageQuery(db);
    languageQuery.prepare("INSERT OR IGNORE INTO languages(code) VALUES (:code)");
    languageQuery.bindValue(":code", language);
    if (!exec(languageQuery, "register migrated language:")) {
      db.rollback();
      return false;
    }
    if (!migrateLegacyTable(db, language, language, std::nullopt)) {
      db.rollback();
      return false;
    }
  }

  dropLegacyTables(db);

  if (!db.commit()) {
    qWarning() << "Failed to commit migration:" << db.lastError();
    return false;
  }
  return true;
}

bool SchemaMigration::ensureSchema(QSqlDatabase& db) {
  QSqlQuery(db).exec("PRAGMA foreign_keys = ON");

  bool hasLegacyTables = false;
  for (const auto& name : allUserTables(db)) {
    if (isLegacyTableName(name)) {
      hasLegacyTables = true;
      break;
    }
  }

  const bool hasItemsTable = tableExists(db, "items");

  if (!hasItemsTable && hasLegacyTables) {
    if (!migrateFromLegacy(db)) {
      return false;
    }
    return upgradeSchema(db);
  }

  if (!hasItemsTable) {
    if (!createSchemaV1(db)) {
      return false;
    }
    return upgradeSchema(db);
  }

  if (!createSchemaV1(db)) {
    return false;
  }

  return upgradeSchema(db);
}

}  // namespace lexis

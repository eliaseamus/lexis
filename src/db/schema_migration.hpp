#pragma once

#include <QSqlDatabase>
#include <QString>

#include <optional>

namespace lexis {

inline constexpr int kSchemaVersion = 3;
inline constexpr int kRootParentId = 0;

class SchemaMigration {
 public:
  static bool ensureSchema(QSqlDatabase& db);
  static bool createSchemaV1(QSqlDatabase& db);
  static bool migrateToV2(QSqlDatabase& db);
  static bool migrateToV3(QSqlDatabase& db);
  static bool upgradeSchema(QSqlDatabase& db);

 private:
  static int currentVersion(QSqlDatabase& db);
  static bool setVersion(QSqlDatabase& db, int version);
  static bool migrateFromLegacy(QSqlDatabase& db);
  static bool isLegacyTableName(const QString& name);
  static QStringList legacyRootTables(QSqlDatabase& db);
  static bool migrateLegacyTable(QSqlDatabase& db, const QString& tableName,
                                 const QString& languageCode,
                                 std::optional<int> parentId);
  static void dropLegacyTables(QSqlDatabase& db);
};

}  // namespace lexis

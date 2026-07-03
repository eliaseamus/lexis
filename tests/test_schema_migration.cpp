#include <QTemporaryDir>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QTest>

#include "schema_migration.hpp"

class SchemaMigrationTest : public QObject {
  Q_OBJECT

 private:
  bool createLegacyDatabase(const QString& path) {
    if (QSqlDatabase::contains("migration_test")) {
      QSqlDatabase::removeDatabase("migration_test");
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "migration_test");
    db.setDatabaseName(path);
    if (!db.open()) {
      return false;
    }

    QSqlQuery query(db);
    if (!query.exec("CREATE TABLE en ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "creation_time TEXT,"
                    "modification_time TEXT,"
                    "title TEXT,"
                    "type INTEGER,"
                    "image BLOB,"
                    "color TEXT,"
                    "audio BLOB,"
                    "meaning TEXT)")) {
      return false;
    }

    query.exec(
      "INSERT INTO en(title, creation_time, modification_time, type, color)"
      "VALUES ('Personality', '2026-01-01', '2026-01-01', 1, '#ffffff')");

    if (!query.exec("CREATE TABLE en_1 ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "parent_table TEXT DEFAULT en,"
                    "parent_id INTEGER DEFAULT 1,"
                    "creation_time TEXT,"
                    "modification_time TEXT,"
                    "title TEXT,"
                    "type INTEGER,"
                    "image BLOB,"
                    "color TEXT,"
                    "audio BLOB,"
                    "meaning TEXT)")) {
      return false;
    }

    query.exec(
      "INSERT INTO en_1(title, creation_time, modification_time, type, color, meaning)"
      "VALUES ('ingenious', '2026-01-02', '2026-01-02', 0, '#a8e6cf', 'clever')");

    db.close();
    QSqlDatabase::removeDatabase("migration_test");
    return true;
  }

 private slots:
  void migratesNestedTablesToNormalizedSchema() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const auto dbPath = tempDir.path() + "/legacy.db";
    QVERIFY(createLegacyDatabase(dbPath));

    {
      QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "migration_test");
      db.setDatabaseName(dbPath);
      QVERIFY(db.open());
      QSqlQuery(db).exec("PRAGMA foreign_keys = ON");
      QVERIFY(lexis::SchemaMigration::ensureSchema(db));
      db.close();
    }
    QSqlDatabase::removeDatabase("migration_test");

    {
      QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "migration_test");
      db.setDatabaseName(dbPath);
      QVERIFY(db.open());

      QSqlQuery versionQuery("SELECT version FROM schema_version", db);
      QVERIFY(versionQuery.exec());
      QVERIFY(versionQuery.next());
      QCOMPARE(versionQuery.value(0).toInt(), lexis::kSchemaVersion);

      QSqlQuery countQuery("SELECT COUNT(*) FROM items", db);
      QVERIFY(countQuery.exec());
      QVERIFY(countQuery.next());
      QCOMPARE(countQuery.value(0).toInt(), 2);

      QSqlQuery wordQuery("SELECT title, meaning, parent_id FROM items WHERE type = 0", db);
      QVERIFY(wordQuery.exec());
      QVERIFY(wordQuery.next());
      QCOMPARE(wordQuery.value("title").toString(), QString("ingenious"));
      QCOMPARE(wordQuery.value("meaning").toString(), QString("clever"));
      QVERIFY(!wordQuery.value("parent_id").isNull());

      QSqlQuery legacyQuery(
        "SELECT name FROM sqlite_master WHERE type = 'table' AND name LIKE 'en%'", db);
      QVERIFY(legacyQuery.exec());
      QVERIFY(!legacyQuery.next());

      db.close();
    }
    QSqlDatabase::removeDatabase("migration_test");
  }
};

QTEST_MAIN(SchemaMigrationTest)
#include "test_schema_migration.moc"

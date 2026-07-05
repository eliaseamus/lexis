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

  void upgradeAddsFrequencyColumns() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const auto dbPath = tempDir.path() + "/v2.db";
    {
      QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "migration_test");
      db.setDatabaseName(dbPath);
      QVERIFY(db.open());

      QSqlQuery query(db);
      QVERIFY(query.exec("CREATE TABLE schema_version (version INTEGER NOT NULL)"));
      QVERIFY(query.exec("INSERT INTO schema_version(version) VALUES (2)"));
      QVERIFY(query.exec("CREATE TABLE languages (code TEXT PRIMARY KEY)"));
      QVERIFY(query.exec("CREATE TABLE items ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                         "language_code TEXT NOT NULL,"
                         "parent_id INTEGER,"
                         "title TEXT NOT NULL,"
                         "type INTEGER NOT NULL,"
                         "creation_time TEXT NOT NULL,"
                         "modification_time TEXT NOT NULL,"
                         "color TEXT,"
                         "meaning TEXT,"
                         "cached_translation TEXT)"));
      db.close();
    }

    {
      QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "migration_test");
      db.setDatabaseName(dbPath);
      QVERIFY(db.open());
      QVERIFY(lexis::SchemaMigration::ensureSchema(db));

      bool hasFrequencyRank = false;
      bool hasFrequencyTier = false;
      QSqlQuery columns("PRAGMA table_info(items)", db);
      while (columns.next()) {
        const auto name = columns.value("name").toString();
        if (name == "frequency_rank") {
          hasFrequencyRank = true;
        } else if (name == "frequency_tier") {
          hasFrequencyTier = true;
        }
      }
      QVERIFY(hasFrequencyRank);
      QVERIFY(hasFrequencyTier);
      db.close();
    }
    QSqlDatabase::removeDatabase("migration_test");
  }

  void upgradeAddsDictionarySummaryColumn() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const auto dbPath = tempDir.path() + "/v3.db";
    {
      QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "migration_test");
      db.setDatabaseName(dbPath);
      QVERIFY(db.open());

      QSqlQuery query(db);
      QVERIFY(query.exec("CREATE TABLE schema_version (version INTEGER NOT NULL)"));
      QVERIFY(query.exec("INSERT INTO schema_version(version) VALUES (3)"));
      QVERIFY(query.exec("CREATE TABLE languages (code TEXT PRIMARY KEY)"));
      QVERIFY(query.exec("CREATE TABLE items ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                         "language_code TEXT NOT NULL,"
                         "parent_id INTEGER,"
                         "title TEXT NOT NULL,"
                         "type INTEGER NOT NULL,"
                         "creation_time TEXT NOT NULL,"
                         "modification_time TEXT NOT NULL,"
                         "color TEXT,"
                         "meaning TEXT,"
                         "cached_translation TEXT,"
                         "frequency_rank INTEGER,"
                         "frequency_tier TEXT)"));
      db.close();
    }

    {
      QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "migration_test");
      db.setDatabaseName(dbPath);
      QVERIFY(db.open());
      QVERIFY(lexis::SchemaMigration::ensureSchema(db));

      bool hasDictionarySummary = false;
      QSqlQuery columns("PRAGMA table_info(items)", db);
      while (columns.next()) {
        if (columns.value("name").toString() == "dictionary_summary") {
          hasDictionarySummary = true;
          break;
        }
      }
      QVERIFY(hasDictionarySummary);
      db.close();
    }
    QSqlDatabase::removeDatabase("migration_test");
  }

  void upgradeAddsSearchFtsIndex() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const auto dbPath = tempDir.path() + "/v4.db";
    {
      QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "migration_test");
      db.setDatabaseName(dbPath);
      QVERIFY(db.open());

      QSqlQuery query(db);
      QVERIFY(query.exec("CREATE TABLE schema_version (version INTEGER NOT NULL)"));
      QVERIFY(query.exec("INSERT INTO schema_version(version) VALUES (4)"));
      QVERIFY(query.exec("CREATE TABLE languages (code TEXT PRIMARY KEY)"));
      QVERIFY(query.exec("CREATE TABLE items ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                         "language_code TEXT NOT NULL,"
                         "parent_id INTEGER,"
                         "title TEXT NOT NULL,"
                         "type INTEGER NOT NULL,"
                         "creation_time TEXT NOT NULL,"
                         "modification_time TEXT NOT NULL,"
                         "color TEXT,"
                         "meaning TEXT,"
                         "cached_translation TEXT,"
                         "dictionary_summary TEXT,"
                         "frequency_rank INTEGER,"
                         "frequency_tier TEXT)"));
      db.close();
    }

    {
      QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "migration_test");
      db.setDatabaseName(dbPath);
      QVERIFY(db.open());
      QVERIFY(lexis::SchemaMigration::ensureSchema(db));

      QSqlQuery ftsQuery(
        "SELECT name FROM sqlite_master WHERE type = 'table' AND name = 'items_fts'", db);
      QVERIFY(ftsQuery.exec());
      QVERIFY(ftsQuery.next());

      QSqlQuery triggerQuery(
        "SELECT name FROM sqlite_master WHERE type = 'trigger' AND name = 'items_fts_insert'", db);
      QVERIFY(triggerQuery.exec());
      QVERIFY(triggerQuery.next());
      db.close();
    }
    QSqlDatabase::removeDatabase("migration_test");
  }
};

QTEST_MAIN(SchemaMigrationTest)
#include "test_schema_migration.moc"

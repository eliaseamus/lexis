#include <QTemporaryDir>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QTest>

#include <optional>

#include "library_archive.hpp"
#include "schema_migration.hpp"

class LibraryArchiveTest : public QObject {
  Q_OBJECT

 private:
  bool insertItem(QSqlDatabase& db, const QString& language, std::optional<int> parentId,
                  const QString& title, int type, const QString& meaning = {}) {
    QSqlQuery query(db);
    query.prepare(
      "INSERT INTO items"
      "(language_code, parent_id, title, creation_time, modification_time, type, color, meaning)"
      "VALUES (:language_code, :parent_id, :title, :creation_time, :modification_time, :type, "
      ":color, :meaning)");
    query.bindValue(":language_code", language);
    if (parentId.has_value()) {
      query.bindValue(":parent_id", *parentId);
    } else {
      query.bindValue(":parent_id", QVariant());
    }
    query.bindValue(":title", title);
    query.bindValue(":creation_time", "2026-01-01T00:00:00");
    query.bindValue(":modification_time", "2026-01-02T00:00:00");
    query.bindValue(":type", type);
    query.bindValue(":color", "#ffffff");
    query.bindValue(":meaning", meaning);
    return query.exec();
  }

 private slots:
  void exportImportRoundTrip() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const auto dbPath = tempDir.path() + "/library.db";
    const auto archivePath = tempDir.path() + "/english.lexis";

    {
      QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "archive_test");
      db.setDatabaseName(dbPath);
      QVERIFY(db.open());
      QSqlQuery(db).exec("PRAGMA foreign_keys = ON");
      QVERIFY(lexis::SchemaMigration::ensureSchema(db));

      QSqlQuery languageQuery(db);
      languageQuery.exec("INSERT INTO languages(code) VALUES ('en')");

      QVERIFY(insertItem(db, "en", std::nullopt, "Personality", 1));
      QSqlQuery lastId(db);
      lastId.exec("SELECT id FROM items WHERE title = 'Personality'");
      QVERIFY(lastId.next());
      const auto parentId = lastId.value(0).toInt();
      QVERIFY(insertItem(db, "en", parentId, "ingenious", 0, "clever"));

      QVERIFY(lexis::LibraryArchive::exportLanguage(db, "en", archivePath));
      db.close();
    }
    QSqlDatabase::removeDatabase("archive_test");

    {
      QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "archive_test");
      db.setDatabaseName(dbPath);
      QVERIFY(db.open());
      QSqlQuery(db).exec("DELETE FROM items");
      QSqlQuery(db).exec("DELETE FROM languages");

      QString importedLanguage;
      QVERIFY(lexis::LibraryArchive::importLanguage(db, archivePath, &importedLanguage));
      QCOMPARE(importedLanguage, QString("en"));

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

      db.close();
    }
    QSqlDatabase::removeDatabase("archive_test");
  }

  void backupAndRestoreDatabase() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const auto dbPath = tempDir.path() + "/library.db";
    const auto backupPath = tempDir.path() + "/backup.db";

    {
      QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "archive_test");
      db.setDatabaseName(dbPath);
      QVERIFY(db.open());
      QVERIFY(lexis::SchemaMigration::ensureSchema(db));
      QSqlQuery(db).exec("INSERT INTO languages(code) VALUES ('de')");
      db.close();
    }
    QSqlDatabase::removeDatabase("archive_test");

    QVERIFY(lexis::LibraryArchive::backupDatabase(dbPath, backupPath));

    {
      QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "archive_test");
      db.setDatabaseName(dbPath);
      QVERIFY(db.open());
      QSqlQuery(db).exec("DELETE FROM languages");
      db.close();
    }
    QSqlDatabase::removeDatabase("archive_test");

    QVERIFY(lexis::LibraryArchive::restoreDatabase(dbPath, backupPath));

    {
      QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "archive_test");
      db.setDatabaseName(dbPath);
      QVERIFY(db.open());

      QSqlQuery query("SELECT code FROM languages", db);
      QVERIFY(query.exec());
      QVERIFY(query.next());
      QCOMPARE(query.value(0).toString(), QString("de"));

      db.close();
    }
    QSqlDatabase::removeDatabase("archive_test");
  }
};

QTEST_MAIN(LibraryArchiveTest)
#include "test_library_archive.moc"

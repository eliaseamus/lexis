#include <QTemporaryDir>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QTest>

#include "library_search.hpp"
#include "schema_migration.hpp"
#include "section_type.hpp"

class LibrarySearchTest : public QObject {
  Q_OBJECT

 private:
  bool insertItem(QSqlDatabase& db, int id, const QString& language, std::optional<int> parentId,
                  const QString& title, int type, const QString& meaning = {}) {
    QSqlQuery query(db);
    query.prepare(
      "INSERT INTO items"
      "(id, language_code, parent_id, title, creation_time, modification_time, type, color, meaning)"
      "VALUES (:id, :language_code, :parent_id, :title, :creation, :modification, :type, :color, "
      ":meaning)");
    query.bindValue(":id", id);
    query.bindValue(":language_code", language);
    if (parentId.has_value()) {
      query.bindValue(":parent_id", *parentId);
    } else {
      query.bindValue(":parent_id", QVariant());
    }
    query.bindValue(":title", title);
    query.bindValue(":creation", "2026-01-01");
    query.bindValue(":modification", "2026-01-01");
    query.bindValue(":type", type);
    query.bindValue(":color", "#ffffff");
    query.bindValue(":meaning", meaning);
    return query.exec();
  }

  bool createDatabase(const QString& path) {
    if (QSqlDatabase::contains("search_test")) {
      QSqlDatabase::removeDatabase("search_test");
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "search_test");
    db.setDatabaseName(path);
    if (!db.open()) {
      return false;
    }

    QSqlQuery(db).exec("PRAGMA foreign_keys = ON");
    if (!lexis::SchemaMigration::ensureSchema(db)) {
      return false;
    }

    QSqlQuery(db).exec("INSERT INTO languages(code) VALUES ('en')");
    if (!insertItem(db, 1, "en", std::nullopt, "Personality", 1)) {
      return false;
    }
    if (!insertItem(db, 2, "en", 1, "Traits", 1)) {
      return false;
    }
    if (!insertItem(db, 3, "en", 2, "ingenious", 0, "clever")) {
      return false;
    }
    if (!insertItem(db, 4, "en", std::nullopt, "Food", 1)) {
      return false;
    }
    if (!insertItem(db, 5, "en", 4, "apple", 0, "a fruit")) {
      return false;
    }

    db.close();
    QSqlDatabase::removeDatabase("search_test");
    return true;
  }

 private slots:
  void findsItemsByTitleAndMeaningAcrossAllLevels() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const auto dbPath = tempDir.path() + "/search.db";
    QVERIFY(createDatabase(dbPath));

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "search_test");
    db.setDatabaseName(dbPath);
    QVERIFY(db.open());

    lexis::SectionTypeManager typeManager;
    const auto titleResults = lexis::LibrarySearch::search(db, "en", "ingenious", typeManager);
    QCOMPARE(titleResults.size(), 1);
    QCOMPARE(titleResults[0].toMap().value("title").toString(), QString("ingenious"));

    const auto meaningResults = lexis::LibrarySearch::search(db, "en", "fruit", typeManager);
    QCOMPARE(meaningResults.size(), 1);
    QCOMPARE(meaningResults[0].toMap().value("title").toString(), QString("apple"));

    db.close();
    QSqlDatabase::removeDatabase("search_test");
  }

  void buildsBreadcrumbAndAncestorPath() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const auto dbPath = tempDir.path() + "/search.db";
    QVERIFY(createDatabase(dbPath));

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "search_test");
    db.setDatabaseName(dbPath);
    QVERIFY(db.open());

    const auto index = lexis::LibrarySearch::loadItemIndex(db, "en");
    QCOMPARE(lexis::LibrarySearch::breadcrumb(index, 3), QString("Personality \u203a Traits \u203a ingenious"));

    const auto path = lexis::LibrarySearch::ancestorPath(index, 3);
    QCOMPARE(path.size(), 3);
    QCOMPARE(path[0].toMap().value("title").toString(), QString("Personality"));
    QCOMPARE(path[1].toMap().value("title").toString(), QString("Traits"));
    QCOMPARE(path[2].toMap().value("title").toString(), QString("ingenious"));

    db.close();
    QSqlDatabase::removeDatabase("search_test");
  }
};

QTEST_MAIN(LibrarySearchTest)
#include "test_library_search.moc"

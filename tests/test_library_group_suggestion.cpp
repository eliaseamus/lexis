#include <QTemporaryDir>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QTest>

#include "library_group_suggestion.hpp"
#include "embedding_lookup.hpp"
#include "schema_migration.hpp"
#include "section_type.hpp"

class LibraryGroupSuggestionTest : public QObject {
  Q_OBJECT

 private:
  bool insertItem(QSqlDatabase& db, int id, const QString& language, std::optional<int> parentId,
                  const QString& title, int type, const QString& meaning = {}) {
    QSqlQuery query(db);
    query.prepare(
      "INSERT INTO items"
      "(id, language_code, parent_id, title, creation_time, modification_time, type, color, "
      "meaning)"
      "VALUES (:id, :language_code, :parent_id, :title, :creation, :modification, :type, "
      ":color, :meaning)");
    query.bindValue(":id", id);
    query.bindValue(":language_code", language);
    if (parentId.has_value()) {
      query.bindValue(":parent_id", *parentId);
    } else {
      query.bindValue(":parent_id", QVariant());
    }
    query.bindValue(":title", title);
    query.bindValue(":creation", "2026-01-01T00:00:00");
    query.bindValue(":modification", "2026-01-01T00:00:00");
    query.bindValue(":type", type);
    query.bindValue(":color", "#ffffff");
    query.bindValue(":meaning", meaning);
    return query.exec();
  }

  bool createDatabase(const QString& path) {
    if (QSqlDatabase::contains("group_suggestion_test")) {
      QSqlDatabase::removeDatabase("group_suggestion_test");
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "group_suggestion_test");
    db.setDatabaseName(path);
    if (!db.open()) {
      return false;
    }

    QSqlQuery(db).exec("PRAGMA foreign_keys = ON");
    if (!lexis::SchemaMigration::ensureSchema(db)) {
      return false;
    }

    QSqlQuery(db).exec("INSERT INTO languages(code) VALUES ('en')");
    using enum lexis::LibrarySectionType;
    if (!insertItem(db, 1, "en", std::nullopt, "Food", std::to_underlying(kSubjectGroup))) {
      return false;
    }
    if (!insertItem(db, 2, "en", 1, "apple", std::to_underlying(kWord), "fruit")) {
      return false;
    }
    if (!insertItem(db, 3, "en", 1, "banana", std::to_underlying(kWord), "fruit")) {
      return false;
    }
    if (!insertItem(db, 4, "en", std::nullopt, "Car", std::to_underlying(kSubjectGroup))) {
      return false;
    }
    if (!insertItem(db, 5, "en", 4, "wheel", std::to_underlying(kWord), "vehicle part")) {
      return false;
    }
    if (!insertItem(db, 6, "en", 4, "engine", std::to_underlying(kWord), "vehicle part")) {
      return false;
    }

    db.close();
    QSqlDatabase::removeDatabase("group_suggestion_test");
    return true;
  }

 private slots:
  void suggestsMatchingSubjectGroupFromExistingWords() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const auto dbPath = tempDir.path() + "/groups.db";
    QVERIFY(createDatabase(dbPath));

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "group_suggestion_test_run");
    db.setDatabaseName(dbPath);
    QVERIFY(db.open());

    const auto suggestions = lexis::LibraryGroupSuggestion::suggestSubjectGroups(
      db, "en", "orange", "fruit", {}, -1, lexis::kRootParentId);

    QCOMPARE(suggestions.size(), 1);
    QCOMPARE(suggestions[0].toMap().value("groupName").toString(), QString("Food"));
    QVERIFY(suggestions[0].toMap().value("confidence").toInt() == 100);

    const auto carSuggestions = lexis::LibraryGroupSuggestion::suggestSubjectGroups(
      db, "en", "engine", {}, {}, -1, lexis::kRootParentId);
    QCOMPARE(carSuggestions.size(), 1);
    QCOMPARE(carSuggestions[0].toMap().value("groupName").toString(), QString("Car"));

    db.close();
    QSqlDatabase::removeDatabase("group_suggestion_test_run");
  }

  void skipsCurrentParentAndLowConfidenceMatches() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const auto dbPath = tempDir.path() + "/groups.db";
    QVERIFY(createDatabase(dbPath));

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "group_suggestion_test_run");
    db.setDatabaseName(dbPath);
    QVERIFY(db.open());

    const auto inCar = lexis::LibraryGroupSuggestion::suggestSubjectGroups(
      db, "en", "wheel", {}, {}, -1, 4);
    QCOMPARE(inCar.size(), 0);

    const auto unrelated = lexis::LibraryGroupSuggestion::suggestSubjectGroups(
      db, "en", "xyz", {}, {}, -1, lexis::kRootParentId);
    QCOMPARE(unrelated.size(), 0);

    db.close();
    QSqlDatabase::removeDatabase("group_suggestion_test_run");
  }

  bool createEmbeddingDatabase(const QString& path) {
    if (QSqlDatabase::contains("embedding_test_setup")) {
      QSqlDatabase::removeDatabase("embedding_test_setup");
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "embedding_test_setup");
    db.setDatabaseName(path);
    if (!db.open()) {
      return false;
    }

    QSqlQuery query(db);
    if (!query.exec("CREATE TABLE metadata (key TEXT PRIMARY KEY, value TEXT NOT NULL)")) {
      return false;
    }
    if (!query.exec("INSERT INTO metadata(key, value) VALUES ('dimensions', '3')")) {
      return false;
    }
    if (!query.exec("CREATE TABLE word_embeddings ("
                    "language_code TEXT NOT NULL,"
                    "word TEXT NOT NULL,"
                    "vector BLOB NOT NULL,"
                    "PRIMARY KEY (language_code, word))")) {
      return false;
    }

    auto insertVector = [&](const QString& word, float x, float y, float z) {
      QSqlQuery insert(db);
      insert.prepare(
        "INSERT INTO word_embeddings(language_code, word, vector) VALUES ('en', :word, :vector)");
      insert.bindValue(":word", word);
      const float values[3] = {x, y, z};
      insert.bindValue(":vector", QByteArray(reinterpret_cast<const char*>(values), sizeof(values)));
      return insert.exec();
    };

    const bool ok = insertVector("apple", 1.0F, 0.0F, 0.0F) &&
                    insertVector("banana", 0.95F, 0.05F, 0.0F) &&
                    insertVector("orange", 0.9F, 0.1F, 0.0F) &&
                    insertVector("wheel", 0.0F, 1.0F, 0.0F) &&
                    insertVector("engine", 0.05F, 0.95F, 0.0F) &&
                    insertVector("steer", 0.0F, 1.0F, 0.0F);

    db.close();
    QSqlDatabase::removeDatabase("embedding_test_setup");
    return ok;
  }

  void prefersSemanticallySimilarGroupWithEmbeddings() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const auto dbPath = tempDir.path() + "/groups.db";
    const auto embeddingPath = tempDir.path() + "/embeddings.db";
    QVERIFY(createDatabase(dbPath));
    QVERIFY(createEmbeddingDatabase(embeddingPath));
    QVERIFY(lexis::EmbeddingLookup::open(embeddingPath));

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "group_suggestion_test_run");
    db.setDatabaseName(dbPath);
    QVERIFY(db.open());

    const auto suggestions = lexis::LibraryGroupSuggestion::suggestSubjectGroups(
      db, "en", "steer", {}, {}, -1, lexis::kRootParentId);
    QVERIFY(!suggestions.isEmpty());
    QCOMPARE(suggestions[0].toMap().value("groupName").toString(), QString("Car"));
    QCOMPARE(suggestions[0].toMap().value("confidence").toInt(), 100);

    db.close();
    QSqlDatabase::removeDatabase("group_suggestion_test_run");
    lexis::EmbeddingLookup::close();
  }
};

QTEST_MAIN(LibraryGroupSuggestionTest)
#include "test_library_group_suggestion.moc"

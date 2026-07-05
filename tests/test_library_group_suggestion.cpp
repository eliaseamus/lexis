#include <QTemporaryDir>
#include <QtEndian>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QTest>

#include <bit>
#include <cmath>

#include "embedding_lookup.hpp"
#include "library_group_suggestion.hpp"
#include "schema_migration.hpp"
#include "section_type.hpp"

namespace {

QByteArray encodeVector(QVector<float> vector) {
  double squaredNorm = 0.0;
  for (const auto component : vector) {
    squaredNorm += static_cast<double>(component) * component;
  }
  const auto norm = static_cast<float>(std::sqrt(squaredNorm));
  float peak = 0.0F;
  for (auto& component : vector) {
    component /= norm;
    peak = std::max(peak, std::abs(component));
  }

  const float scale = peak / 127.0F;
  QByteArray blob;
  const auto scaleBits = qToLittleEndian(std::bit_cast<quint32>(scale));
  blob.append(reinterpret_cast<const char*>(&scaleBits), 4);
  for (const auto component : vector) {
    blob.append(static_cast<char>(qRound(component / scale)));
  }
  return blob;
}

}  // namespace

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
    QVERIFY(suggestions[0].toMap().value("confidence").toInt() >= 40);

    const auto carSuggestions = lexis::LibraryGroupSuggestion::suggestSubjectGroups(
      db, "en", "engine", {}, {}, -1, lexis::kRootParentId);
    QCOMPARE(carSuggestions.size(), 1);
    QCOMPARE(carSuggestions[0].toMap().value("groupName").toString(), QString("Car"));

    db.close();
    QSqlDatabase::removeDatabase("group_suggestion_test_run");
  }

  void suggestsGroupSemanticallyThroughEmbeddings() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const auto dbPath = tempDir.path() + "/groups.db";
    const auto embeddingPath = tempDir.path() + "/embeddings.db";

    // Library: a "Weapon" subject group holding sword and dagger.
    {
      QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "group_suggestion_test");
      db.setDatabaseName(dbPath);
      QVERIFY(db.open());
      QVERIFY(lexis::SchemaMigration::ensureSchema(db));
      QSqlQuery(db).exec("INSERT INTO languages(code) VALUES ('en')");
      using enum lexis::LibrarySectionType;
      QVERIFY(insertItem(db, 1, "en", std::nullopt, "Weapon", std::to_underlying(kSubjectGroup)));
      QVERIFY(insertItem(db, 2, "en", 1, "sword", std::to_underlying(kWord)));
      QVERIFY(insertItem(db, 3, "en", 1, "dagger", std::to_underlying(kWord)));
      db.close();
    }
    QSqlDatabase::removeDatabase("group_suggestion_test");

    // Synthetic embeddings placing spear next to sword/dagger/weapon.
    {
      QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "embedding_fixture");
      db.setDatabaseName(embeddingPath);
      QVERIFY(db.open());
      QSqlQuery(db).exec(
        "CREATE TABLE word_embeddings ("
        "language_code TEXT NOT NULL, word TEXT NOT NULL, rank INTEGER NOT NULL, "
        "vector BLOB NOT NULL, PRIMARY KEY (language_code, word))");
      const auto insert = [&db](const QString& word, const QVector<float>& vector) {
        QSqlQuery query(db);
        query.prepare(
          "INSERT INTO word_embeddings(language_code, word, rank, vector) "
          "VALUES ('en', :word, 1000, :vector)");
        query.bindValue(":word", word);
        query.bindValue(":vector", encodeVector(vector));
        return query.exec();
      };
      QVERIFY(insert("weapon", {0.95F, 0.05F, 0.0F}));
      QVERIFY(insert("spear", {0.90F, 0.10F, 0.0F}));
      QVERIFY(insert("sword", {0.92F, 0.05F, 0.0F}));
      QVERIFY(insert("dagger", {0.88F, 0.12F, 0.0F}));
      db.close();
    }
    QSqlDatabase::removeDatabase("embedding_fixture");
    QVERIFY(lexis::EmbeddingLookup::open(embeddingPath));

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "group_suggestion_test_run");
    db.setDatabaseName(dbPath);
    QVERIFY(db.open());

    // No lexical overlap between "spear" and the group, so any suggestion
    // must come from embedding similarity.
    const auto suggestions = lexis::LibraryGroupSuggestion::suggestSubjectGroups(
      db, "en", "spear", {}, {}, -1, lexis::kRootParentId);

    QCOMPARE(suggestions.size(), 1);
    QCOMPARE(suggestions[0].toMap().value("groupName").toString(), QString("Weapon"));
    QVERIFY(suggestions[0].toMap().value("confidence").toInt() >= 40);

    db.close();
    QSqlDatabase::removeDatabase("group_suggestion_test_run");
    lexis::EmbeddingLookup::close();
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
};

QTEST_MAIN(LibraryGroupSuggestionTest)
#include "test_library_group_suggestion.moc"

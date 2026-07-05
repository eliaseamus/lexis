#include <QTemporaryDir>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QTest>

#include "embedding_lookup.hpp"
#include "text_embedding.hpp"

class EmbeddingLookupTest : public QObject {
  Q_OBJECT

 private:
  bool createSampleDatabase(const QString& path) {
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

    if (!insertVector("apple", 1.0F, 0.0F, 0.0F)) {
      return false;
    }
    if (!insertVector("banana", 0.9F, 0.1F, 0.0F)) {
      return false;
    }
    if (!insertVector("engine", 0.0F, 1.0F, 0.0F)) {
      return false;
    }
    if (!insertVector("wheel", 0.1F, 0.9F, 0.0F)) {
      return false;
    }

    db.close();
    QSqlDatabase::removeDatabase("embedding_test_setup");
    return true;
  }

 private slots:
  void lookupReturnsStoredVector() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const auto dbPath = tempDir.path() + "/embeddings.db";
    QVERIFY(createSampleDatabase(dbPath));

    QVERIFY(lexis::EmbeddingLookup::open(dbPath));
    QCOMPARE(lexis::EmbeddingLookup::dimensions(), 3);

    const auto apple = lexis::EmbeddingLookup::lookup("en", "apple");
    QVERIFY(apple.found);
    QCOMPARE(apple.values.size(), 3);
    QVERIFY(qAbs(apple.values[0] - 1.0F) < 0.001F);

    lexis::EmbeddingLookup::close();
  }

  void cosineSimilarityRanksRelatedWordsHigher() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const auto dbPath = tempDir.path() + "/embeddings.db";
    QVERIFY(createSampleDatabase(dbPath));
    QVERIFY(lexis::EmbeddingLookup::open(dbPath));

    const auto apple = lexis::EmbeddingLookup::lookup("en", "apple");
    const auto banana = lexis::EmbeddingLookup::lookup("en", "banana");
    const auto engine = lexis::EmbeddingLookup::lookup("en", "engine");
    QVERIFY(apple.found && banana.found && engine.found);

    const auto fruitSimilarity = lexis::EmbeddingLookup::cosineSimilarity(apple.values, banana.values);
    const auto crossSimilarity = lexis::EmbeddingLookup::cosineSimilarity(apple.values, engine.values);
    QVERIFY(fruitSimilarity > crossSimilarity);

    lexis::EmbeddingLookup::close();
  }

  void textEmbeddingAveragesTokens() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const auto dbPath = tempDir.path() + "/embeddings.db";
    QVERIFY(createSampleDatabase(dbPath));
    QVERIFY(lexis::EmbeddingLookup::open(dbPath));

    const auto vehicle = lexis::TextEmbedding::embed("en", "engine wheel");
    QVERIFY(vehicle.has_value());
    const auto engine = lexis::EmbeddingLookup::lookup("en", "engine");
    const auto wheel = lexis::EmbeddingLookup::lookup("en", "wheel");
    const auto expectedX = (engine.values[0] + wheel.values[0]) / 2.0F;
    QVERIFY(qAbs((*vehicle)[0] - expectedX) < 0.001F);

    lexis::EmbeddingLookup::close();
  }
};

QTEST_MAIN(EmbeddingLookupTest)
#include "test_embedding_lookup.moc"

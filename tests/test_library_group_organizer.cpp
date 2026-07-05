#include <QTemporaryDir>
#include <QtEndian>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QTest>

#include <algorithm>
#include <bit>
#include <cmath>

#include "embedding_lookup.hpp"
#include "library_group_organizer.hpp"
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

class LibraryGroupOrganizerTest : public QObject {
  Q_OBJECT

 private:
  QTemporaryDir _tempDir;

  bool insertEmbedding(QSqlDatabase& db, const QString& word, int rank,
                       const QVector<float>& vector) {
    QSqlQuery query(db);
    query.prepare(
      "INSERT INTO word_embeddings(language_code, word, rank, vector) "
      "VALUES ('en', :word, :rank, :vector)");
    query.bindValue(":word", word);
    query.bindValue(":rank", rank);
    query.bindValue(":vector", encodeVector(vector));
    return query.exec();
  }

  bool createEmbeddingDatabase(const QString& path) {
    {
      QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "embedding_fixture");
      db.setDatabaseName(path);
      if (!db.open()) {
        return false;
      }

      QSqlQuery(db).exec(
        "CREATE TABLE word_embeddings ("
        "language_code TEXT NOT NULL, word TEXT NOT NULL, rank INTEGER NOT NULL, "
        "vector BLOB NOT NULL, PRIMARY KEY (language_code, word))");

      // Label candidates (rank must exceed the function-word cutoff of 200).
      bool ok = insertEmbedding(db, "fruit", 250, {0.0F, 0.95F, 0.05F, 0.0F}) &&
                insertEmbedding(db, "weapon", 300, {0.95F, 0.05F, 0.0F, 0.0F}) &&
                // Weapon-like words.
                insertEmbedding(db, "spear", 5000, {0.90F, 0.10F, 0.0F, 0.0F}) &&
                insertEmbedding(db, "sword", 5100, {0.92F, 0.05F, 0.0F, 0.0F}) &&
                insertEmbedding(db, "dagger", 5200, {0.88F, 0.12F, 0.0F, 0.0F}) &&
                // Fruit-like words.
                insertEmbedding(db, "apple", 900, {0.0F, 0.90F, 0.10F, 0.0F}) &&
                insertEmbedding(db, "banana", 1500, {0.0F, 0.92F, 0.08F, 0.0F}) &&
                // An unrelated word that must stay unclustered.
                insertEmbedding(db, "notebook", 2000, {0.0F, 0.0F, 0.05F, 0.95F});
      db.close();
      if (!ok) {
        return false;
      }
    }
    QSqlDatabase::removeDatabase("embedding_fixture");
    return true;
  }

  bool insertWord(QSqlDatabase& db, int id, const QString& title) {
    QSqlQuery query(db);
    query.prepare(
      "INSERT INTO items"
      "(id, language_code, parent_id, title, creation_time, modification_time, type, color)"
      "VALUES (:id, 'en', NULL, :title, '2026-01-01T00:00:00', '2026-01-01T00:00:00', "
      ":type, '#ffffff')");
    query.bindValue(":id", id);
    query.bindValue(":title", title);
    query.bindValue(":type", std::to_underlying(lexis::LibrarySectionType::kWord));
    return query.exec();
  }

  bool createLibraryDatabase(const QString& path) {
    {
      QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "library_fixture");
      db.setDatabaseName(path);
      if (!db.open()) {
        return false;
      }
      if (!lexis::SchemaMigration::ensureSchema(db)) {
        return false;
      }
      QSqlQuery(db).exec("INSERT INTO languages(code) VALUES ('en')");
      const bool ok = insertWord(db, 1, "spear") && insertWord(db, 2, "sword") &&
                      insertWord(db, 3, "dagger") && insertWord(db, 4, "apple") &&
                      insertWord(db, 5, "banana") && insertWord(db, 6, "notebook");
      db.close();
      if (!ok) {
        return false;
      }
    }
    QSqlDatabase::removeDatabase("library_fixture");
    return true;
  }

 private slots:
  void returnsNothingWithoutEmbeddingDatabase() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const auto dbPath = tempDir.path() + "/library.db";
    QVERIFY(createLibraryDatabase(dbPath));

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "organizer_run");
    db.setDatabaseName(dbPath);
    QVERIFY(db.open());

    QVERIFY(!lexis::EmbeddingLookup::isOpen());
    const auto proposals =
      lexis::LibraryGroupOrganizer::proposeGroups(db, "en", lexis::kRootParentId);
    QVERIFY(proposals.isEmpty());

    db.close();
    QSqlDatabase::removeDatabase("organizer_run");
  }

  void clustersRelatedWordsAndNamesGroups() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const auto libraryPath = tempDir.path() + "/library.db";
    const auto embeddingPath = tempDir.path() + "/embeddings.db";
    QVERIFY(createLibraryDatabase(libraryPath));
    QVERIFY(createEmbeddingDatabase(embeddingPath));
    QVERIFY(lexis::EmbeddingLookup::open(embeddingPath));

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "organizer_run");
    db.setDatabaseName(libraryPath);
    QVERIFY(db.open());

    const auto proposals =
      lexis::LibraryGroupOrganizer::proposeGroups(db, "en", lexis::kRootParentId);
    QCOMPARE(proposals.size(), 2);

    const auto weapons = proposals[0].toMap();
    QCOMPARE(weapons.value("name").toString(), QString("Weapon"));
    QCOMPARE(weapons.value("words").toList().size(), 3);
    QVERIFY(weapons.value("confidence").toInt() > 50);

    const auto fruits = proposals[1].toMap();
    QCOMPARE(fruits.value("name").toString(), QString("Fruit"));
    QCOMPARE(fruits.value("words").toList().size(), 2);

    QStringList weaponTitles;
    for (const auto& word : weapons.value("words").toList()) {
      weaponTitles.append(word.toMap().value("title").toString());
    }
    std::sort(weaponTitles.begin(), weaponTitles.end());
    QCOMPARE(weaponTitles, QStringList({"dagger", "spear", "sword"}));

    db.close();
    QSqlDatabase::removeDatabase("organizer_run");
    lexis::EmbeddingLookup::close();
  }
};

QTEST_MAIN(LibraryGroupOrganizerTest)
#include "test_library_group_organizer.moc"

#include <QTemporaryDir>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QTest>

#include "frequency_lookup.hpp"

class FrequencyLookupTest : public QObject {
  Q_OBJECT

 private slots:
  void lookupReturnsTierForKnownEnglishWord() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const auto dbPath = tempDir.path() + "/frequency.db";
    QVERIFY(createSampleDatabase(dbPath));

    QVERIFY(lexis::FrequencyLookup::open(dbPath));
    const auto result = lexis::FrequencyLookup::lookup("en", "hello");
    QVERIFY(result.found);
    QCOMPARE(result.tier, QString("core"));
    QVERIFY(result.rank > 0);
    QVERIFY(result.rank <= 5000);
    QVERIFY(result.zipf > 0.0);
    lexis::FrequencyLookup::close();
  }

  void lookupIsCaseInsensitive() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const auto dbPath = tempDir.path() + "/frequency.db";
    QVERIFY(createSampleDatabase(dbPath));

    QVERIFY(lexis::FrequencyLookup::open(dbPath));
    const auto lower = lexis::FrequencyLookup::lookup("en", "hello");
    const auto upper = lexis::FrequencyLookup::lookup("en", "HELLO");
    QVERIFY(lower.found);
    QVERIFY(upper.found);
    QCOMPARE(lower.rank, upper.rank);
    QCOMPARE(lower.tier, upper.tier);
    lexis::FrequencyLookup::close();
  }

  void lookupReturnsEmptyForUnknownWord() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const auto dbPath = tempDir.path() + "/frequency.db";
    QVERIFY(createSampleDatabase(dbPath));

    QVERIFY(lexis::FrequencyLookup::open(dbPath));
    const auto result = lexis::FrequencyLookup::lookup("en", "notawordxyz123");
    QVERIFY(!result.found);
    lexis::FrequencyLookup::close();
  }

  void tierFromZipfUsesLexisBands() {
    QCOMPARE(lexis::FrequencyLookup::tierFromZipf(5.72), QString("core"));
    QCOMPARE(lexis::FrequencyLookup::tierFromZipf(3.89), QString("common"));
    QCOMPARE(lexis::FrequencyLookup::tierFromZipf(2.36), QString("intermediate"));
    QCOMPARE(lexis::FrequencyLookup::tierFromZipf(1.5), QString("advanced"));
    QCOMPARE(lexis::FrequencyLookup::tierFromZipf(0.5), QString("rare"));
  }

 private:
  bool createSampleDatabase(const QString& path) {
    if (QSqlDatabase::contains("freq_test_setup")) {
      QSqlDatabase::removeDatabase("freq_test_setup");
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "freq_test_setup");
    db.setDatabaseName(path);
    if (!db.open()) {
      return false;
    }

    QSqlQuery query(db);
    if (!query.exec(
          "CREATE TABLE word_frequencies ("
          "language_code TEXT NOT NULL, word TEXT NOT NULL, rank INTEGER NOT NULL, "
          "zipf REAL NOT NULL, tier TEXT NOT NULL, PRIMARY KEY (language_code, word))")) {
      db.close();
      QSqlDatabase::removeDatabase("freq_test_setup");
      return false;
    }

    query.prepare(
      "INSERT INTO word_frequencies(language_code, word, rank, zipf, tier) "
      "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue("en");
    query.addBindValue("hello");
    query.addBindValue(120);
    query.addBindValue(6.2);
    query.addBindValue("core");
    if (!query.exec()) {
      db.close();
      QSqlDatabase::removeDatabase("freq_test_setup");
      return false;
    }

    db.close();
    QSqlDatabase::removeDatabase("freq_test_setup");
    return true;
  }
};

QTEST_MAIN(FrequencyLookupTest)
#include "test_frequency_lookup.moc"

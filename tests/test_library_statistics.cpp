#include <QTemporaryDir>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QTest>

#include "library_statistics.hpp"
#include "schema_migration.hpp"
#include "section_type.hpp"
#include "utils.hpp"

class LibraryStatisticsTest : public QObject {
  Q_OBJECT

 private:
  bool insertItem(QSqlDatabase& db, int id, const QString& language, std::optional<int> parentId,
                  const QString& title, int type, const QString& meaning = {},
                  const QByteArray& image = {}, const QByteArray& audio = {},
                  const QString& creationTime = "2026-01-01T00:00:00") {
    QSqlQuery query(db);
    query.prepare(
      "INSERT INTO items"
      "(id, language_code, parent_id, title, creation_time, modification_time, type, color, "
      "meaning, image, audio)"
      "VALUES (:id, :language_code, :parent_id, :title, :creation, :modification, :type, "
      ":color, :meaning, :image, :audio)");
    query.bindValue(":id", id);
    query.bindValue(":language_code", language);
    if (parentId.has_value()) {
      query.bindValue(":parent_id", *parentId);
    } else {
      query.bindValue(":parent_id", QVariant());
    }
    query.bindValue(":title", title);
    query.bindValue(":creation", creationTime);
    query.bindValue(":modification", creationTime);
    query.bindValue(":type", type);
    query.bindValue(":color", "#ffffff");
    query.bindValue(":meaning", meaning);
    query.bindValue(":image", image);
    query.bindValue(":audio", audio);
    return query.exec();
  }

  bool createDatabase(const QString& path) {
    if (QSqlDatabase::contains("stats_test")) {
      QSqlDatabase::removeDatabase("stats_test");
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "stats_test");
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
    if (!insertItem(db, 3, "en", 2, "ingenious", 0, "clever", QByteArray("img"))) {
      return false;
    }
    if (!insertItem(db, 4, "en", std::nullopt, "Food", 1)) {
      return false;
    }
    if (!insertItem(db, 5, "en", 4, "apple", 0)) {
      return false;
    }
    if (!insertItem(db, 6, "en", std::nullopt, "apple", 0, "duplicate")) {
      return false;
    }

    db.close();
    QSqlDatabase::removeDatabase("stats_test");
    return true;
  }

 private slots:
  void languageStatsCountsLibraryTotals() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const auto dbPath = tempDir.path() + "/stats.db";
    QVERIFY(createDatabase(dbPath));

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "stats_test_run");
    db.setDatabaseName(dbPath);
    QVERIFY(db.open());

    lexis::SectionTypeManager typeManager;
    const auto stats = lexis::LibraryStatistics::languageStats(db, "en", typeManager);

    QCOMPARE(stats.value("totalItems").toInt(), 6);
    QCOMPARE(stats.value("wordCount").toInt(), 3);
    QCOMPARE(stats.value("subjectGroupCount").toInt(), 3);

    const auto wordsByCategory = stats.value("wordsByCategory").toList();
    QCOMPARE(wordsByCategory.size(), 3);

    QHash<QString, int> categoryCounts;
    for (const auto& entryValue : wordsByCategory) {
      const auto entry = entryValue.toMap();
      categoryCounts.insert(entry.value("categoryName").toString(), entry.value("count").toInt());
    }
    QCOMPARE(categoryCounts.value("Personality"), 1);
    QCOMPARE(categoryCounts.value("Food"), 1);
    QCOMPARE(categoryCounts.value("Standalone"), 1);
    QCOMPARE(categoryCounts.contains("Traits"), false);

    db.close();
    QSqlDatabase::removeDatabase("stats_test_run");
  }

  void itemStatsIncludesWordsBySubjectGroupForContainers() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const auto dbPath = tempDir.path() + "/stats.db";
    QVERIFY(createDatabase(dbPath));

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "stats_test_run");
    db.setDatabaseName(dbPath);
    QVERIFY(db.open());

    lexis::SectionTypeManager typeManager;
    const auto personalityStats = lexis::LibraryStatistics::itemStats(db, "en", 1, typeManager);

    const auto wordsByCategory = personalityStats.value("wordsByCategory").toList();
    QCOMPARE(wordsByCategory.size(), 1);
    QCOMPARE(wordsByCategory[0].toMap().value("categoryName").toString(), QString("Traits"));
    QCOMPARE(wordsByCategory[0].toMap().value("count").toInt(), 1);

    const auto wordStats = lexis::LibraryStatistics::itemStats(db, "en", 3, typeManager);
    QVERIFY(!wordStats.contains("wordsByCategory"));

    db.close();
    QSqlDatabase::removeDatabase("stats_test_run");
  }

  void itemStatsIncludesHierarchyAndMetadata() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const auto dbPath = tempDir.path() + "/stats.db";
    QVERIFY(createDatabase(dbPath));

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "stats_test_run");
    db.setDatabaseName(dbPath);
    QVERIFY(db.open());

    lexis::SectionTypeManager typeManager;
    const auto stats = lexis::LibraryStatistics::itemStats(db, "en", 1, typeManager);

    QCOMPARE(stats.value("title").toString(), QString("Personality"));
    QCOMPARE(stats.value("typeName").toString(), QString("Subject group"));
    QCOMPARE(stats.value("directChildren").toInt(), 1);
    QCOMPARE(stats.value("descendants").toInt(), 2);
    QCOMPARE(stats.value("breadcrumb").toString(), QString());

    const auto wordStats = lexis::LibraryStatistics::itemStats(db, "en", 3, typeManager);
    QCOMPARE(wordStats.value("breadcrumb").toString(), QString("Personality \u203a Traits"));
    QCOMPARE(wordStats.value("directChildren").toInt(), 0);
    QCOMPARE(wordStats.value("descendants").toInt(), 0);

    db.close();
    QSqlDatabase::removeDatabase("stats_test_run");
  }

  void languageStatsCountsRecentlyAddedItemsWithTextDateFormat() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const auto dbPath = tempDir.path() + "/stats.db";
    QVERIFY(createDatabase(dbPath));

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "stats_test_run");
    db.setDatabaseName(dbPath);
    QVERIFY(db.open());

    const auto recentCreationTime = QDateTime::currentDateTime().toString(Qt::TextDate);
    QVERIFY(insertItem(db, 7, "en", 4, "Steer", 0, {}, {}, {}, recentCreationTime));

    lexis::SectionTypeManager typeManager;
    const auto stats = lexis::LibraryStatistics::languageStats(db, "en", typeManager);

    QCOMPARE(stats.value("totalItems").toInt(), 7);
    QCOMPARE(stats.value("addedThisMonth").toInt(), 1);
    QCOMPARE(stats.value("addedLast7Days").toInt(), 1);
    QCOMPARE(stats.value("addedLast30Days").toInt(), 1);

    db.close();
    QSqlDatabase::removeDatabase("stats_test_run");
  }

  void scopedWordsReturnsAllWordsForLanguage() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const auto dbPath = tempDir.path() + "/stats.db";
    QVERIFY(createDatabase(dbPath));

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "stats_test_run");
    db.setDatabaseName(dbPath);
    QVERIFY(db.open());

    const auto words = lexis::LibraryStatistics::scopedWords(db, "en", lexis::kRootParentId);
    QCOMPARE(words.size(), 3);

    QHash<QString, QVariantMap> wordsByTitle;
    for (const auto& entryValue : words) {
      const auto entry = entryValue.toMap();
      wordsByTitle.insert(entry.value("title").toString(), entry);
    }

    QCOMPARE(wordsByTitle.value("ingenious").value("meaning").toString(), QString("clever"));
    QCOMPARE(wordsByTitle.value("ingenious").value("hasImage").toBool(), true);
    QCOMPARE(wordsByTitle.value("apple").value("hasImage").toBool(), false);

    db.close();
    QSqlDatabase::removeDatabase("stats_test_run");
  }

  void scopedWordsReturnsSubtreeWordsOnly() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const auto dbPath = tempDir.path() + "/stats.db";
    QVERIFY(createDatabase(dbPath));

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "stats_test_run");
    db.setDatabaseName(dbPath);
    QVERIFY(db.open());

    const auto personalityWords = lexis::LibraryStatistics::scopedWords(db, "en", 1);
    QCOMPARE(personalityWords.size(), 1);
    QCOMPARE(personalityWords[0].toMap().value("title").toString(), QString("ingenious"));

    const auto foodWords = lexis::LibraryStatistics::scopedWords(db, "en", 4);
    QCOMPARE(foodWords.size(), 1);
    QCOMPARE(foodWords[0].toMap().value("title").toString(), QString("apple"));

    db.close();
    QSqlDatabase::removeDatabase("stats_test_run");
  }
};

QTEST_MAIN(LibraryStatisticsTest)
#include "test_library_statistics.moc"

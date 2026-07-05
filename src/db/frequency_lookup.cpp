#include "frequency_lookup.hpp"

#include <QFile>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

namespace lexis {

namespace {

QString g_databasePath;
constexpr auto kConnectionName = "lexis_frequency";

}  // namespace

bool FrequencyLookup::open(const QString& databasePath) {
  const QFileInfo fileInfo(databasePath);
  if (!fileInfo.exists() || !fileInfo.isFile()) {
    qWarning() << "Frequency database not found:" << databasePath;
    return false;
  }

  if (QSqlDatabase::contains(kConnectionName)) {
    QSqlDatabase::removeDatabase(kConnectionName);
  }

  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", kConnectionName);
  db.setDatabaseName(fileInfo.absoluteFilePath());
  if (!db.open()) {
    qWarning() << "Failed to open frequency database:" << db.lastError();
    QSqlDatabase::removeDatabase(kConnectionName);
    return false;
  }

  g_databasePath = fileInfo.absoluteFilePath();
  return true;
}

void FrequencyLookup::close() {
  if (QSqlDatabase::contains(kConnectionName)) {
    QSqlDatabase db = QSqlDatabase::database(kConnectionName);
    if (db.isOpen()) {
      db.close();
    }
    QSqlDatabase::removeDatabase(kConnectionName);
  }
  g_databasePath.clear();
}

bool FrequencyLookup::isOpen() {
  return QSqlDatabase::contains(kConnectionName) &&
         QSqlDatabase::database(kConnectionName).isOpen();
}

QString FrequencyLookup::databasePath() {
  return g_databasePath;
}

QString FrequencyLookup::normalizeWord(const QString& word) {
  return word.trimmed().toCaseFolded();
}

QString FrequencyLookup::tierFromZipf(double zipf) {
  if (zipf >= 4.0) {
    return QStringLiteral("core");
  }
  if (zipf >= 3.0) {
    return QStringLiteral("common");
  }
  if (zipf >= 2.0) {
    return QStringLiteral("intermediate");
  }
  if (zipf >= 1.0) {
    return QStringLiteral("advanced");
  }
  return QStringLiteral("rare");
}

QSqlDatabase FrequencyLookup::connection() {
  return QSqlDatabase::database(kConnectionName);
}

FrequencyLookup::Result FrequencyLookup::lookup(const QString& languageCode,
                                                const QString& word) {
  Result result;
  if (!isOpen() || languageCode.isEmpty()) {
    return result;
  }

  const auto normalizedWord = normalizeWord(word);
  if (normalizedWord.isEmpty()) {
    return result;
  }

  QSqlQuery query(connection());
  query.prepare(
    "SELECT rank, zipf, tier FROM word_frequencies "
    "WHERE language_code = :language_code AND word = :word");
  query.bindValue(":language_code", languageCode);
  query.bindValue(":word", normalizedWord);

  if (!query.exec()) {
    qWarning() << "Frequency lookup failed:" << query.lastError();
    return result;
  }

  if (!query.next()) {
    return result;
  }

  result.rank = query.value("rank").toInt();
  result.zipf = query.value("zipf").toDouble();
  result.tier = tierFromZipf(result.zipf);
  result.found = result.rank > 0 && result.zipf > 0.0 && !result.tier.isEmpty();
  return result;
}

QVariantMap FrequencyLookup::lookupMap(const QString& languageCode, const QString& word) {
  const auto result = lookup(languageCode, word);
  if (!result.found) {
    return {};
  }

  return QVariantMap{
    {QStringLiteral("rank"), result.rank},
    {QStringLiteral("zipf"), result.zipf },
    {QStringLiteral("tier"), result.tier },
  };
}

}  // namespace lexis

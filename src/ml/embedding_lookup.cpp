#include "embedding_lookup.hpp"

#include <QDebug>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QtEndian>

#include <bit>

namespace lexis {

namespace {

QString g_databasePath;
QHash<QString, EmbeddingVector> g_vectorCache;
constexpr auto kConnectionName = "lexis_embeddings";
constexpr int kMaxCachedVectors = 50'000;

QString cacheKey(const QString& languageCode, const QString& word) {
  return languageCode + QChar(0x1F) + word;
}

}  // namespace

bool EmbeddingLookup::open(const QString& databasePath) {
  const QFileInfo fileInfo(databasePath);
  if (!fileInfo.exists() || !fileInfo.isFile()) {
    qWarning() << "Embedding database not found:" << databasePath;
    return false;
  }

  if (QSqlDatabase::contains(kConnectionName)) {
    QSqlDatabase::removeDatabase(kConnectionName);
  }

  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", kConnectionName);
  db.setDatabaseName(fileInfo.absoluteFilePath());
  if (!db.open()) {
    qWarning() << "Failed to open embedding database:" << db.lastError();
    QSqlDatabase::removeDatabase(kConnectionName);
    return false;
  }

  g_databasePath = fileInfo.absoluteFilePath();
  g_vectorCache.clear();
  return true;
}

void EmbeddingLookup::close() {
  if (QSqlDatabase::contains(kConnectionName)) {
    QSqlDatabase db = QSqlDatabase::database(kConnectionName);
    if (db.isOpen()) {
      db.close();
    }
    QSqlDatabase::removeDatabase(kConnectionName);
  }
  g_databasePath.clear();
  g_vectorCache.clear();
}

bool EmbeddingLookup::isOpen() {
  return QSqlDatabase::contains(kConnectionName) &&
         QSqlDatabase::database(kConnectionName).isOpen();
}

QString EmbeddingLookup::databasePath() {
  return g_databasePath;
}

QSqlDatabase EmbeddingLookup::connection() {
  return QSqlDatabase::database(kConnectionName);
}

EmbeddingVector EmbeddingLookup::decodeVector(const QByteArray& blob) {
  if (blob.size() < 5) {
    return {};
  }

  const auto scaleBits = qFromLittleEndian<quint32>(blob.constData());
  const auto scale = std::bit_cast<float>(scaleBits);
  if (!(scale > 0.0F)) {
    return {};
  }

  const int dimensions = blob.size() - 4;
  EmbeddingVector vector(dimensions);
  const auto* components = reinterpret_cast<const signed char*>(blob.constData() + 4);
  for (int i = 0; i < dimensions; ++i) {
    vector[i] = scale * static_cast<float>(components[i]);
  }
  return vector;
}

EmbeddingVector EmbeddingLookup::lookup(const QString& languageCode, const QString& word) {
  if (!isOpen() || languageCode.isEmpty()) {
    return {};
  }

  const auto normalizedWord = word.trimmed().toCaseFolded();
  if (normalizedWord.isEmpty()) {
    return {};
  }

  const auto key = cacheKey(languageCode, normalizedWord);
  const auto cached = g_vectorCache.constFind(key);
  if (cached != g_vectorCache.constEnd()) {
    return *cached;
  }

  QSqlQuery query(connection());
  query.prepare(
    "SELECT vector FROM word_embeddings "
    "WHERE language_code = :language_code AND word = :word");
  query.bindValue(":language_code", languageCode);
  query.bindValue(":word", normalizedWord);

  EmbeddingVector vector;
  if (!query.exec()) {
    qWarning() << "Embedding lookup failed:" << query.lastError();
    return vector;
  }
  if (query.next()) {
    vector = decodeVector(query.value(0).toByteArray());
  }

  if (g_vectorCache.size() >= kMaxCachedVectors) {
    g_vectorCache.clear();
  }
  g_vectorCache.insert(key, vector);
  return vector;
}

QVector<EmbeddingLookup::RankedWord> EmbeddingLookup::topWords(const QString& languageCode,
                                                               int limit) {
  QVector<RankedWord> words;
  if (!isOpen() || languageCode.isEmpty() || limit <= 0) {
    return words;
  }

  QSqlQuery query(connection());
  query.prepare(
    "SELECT word, rank, vector FROM word_embeddings "
    "WHERE language_code = :language_code ORDER BY rank LIMIT :limit");
  query.bindValue(":language_code", languageCode);
  query.bindValue(":limit", limit);

  if (!query.exec()) {
    qWarning() << "Embedding top-words query failed:" << query.lastError();
    return words;
  }

  words.reserve(limit);
  while (query.next()) {
    auto vector = decodeVector(query.value(2).toByteArray());
    if (vector.isEmpty()) {
      continue;
    }
    words.append({query.value(0).toString(), query.value(1).toInt(), std::move(vector)});
  }
  return words;
}

}  // namespace lexis

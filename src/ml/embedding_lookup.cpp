#include "embedding_lookup.hpp"

#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>

#include <cmath>
#include <cstring>

namespace lexis {

namespace {

QString g_databasePath;
int g_dimensions = 0;
constexpr auto kConnectionName = "lexis_embeddings";

}  // namespace

bool EmbeddingLookup::open(const QString& databasePath) {
  const QFileInfo fileInfo(databasePath);
  if (!fileInfo.exists() || !fileInfo.isFile()) {
    qWarning() << "Embeddings database not found:" << databasePath;
    return false;
  }

  if (QSqlDatabase::contains(kConnectionName)) {
    QSqlDatabase::removeDatabase(kConnectionName);
  }

  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", kConnectionName);
  db.setDatabaseName(fileInfo.absoluteFilePath());
  if (!db.open()) {
    qWarning() << "Failed to open embeddings database:" << db.lastError();
    QSqlDatabase::removeDatabase(kConnectionName);
    return false;
  }

  QSqlQuery query(db);
  query.prepare("SELECT value FROM metadata WHERE key = :key");
  query.bindValue(":key", QStringLiteral("dimensions"));
  if (!query.exec() || !query.next()) {
    qWarning() << "Embeddings database is missing dimensions metadata";
    db.close();
    QSqlDatabase::removeDatabase(kConnectionName);
    return false;
  }

  g_dimensions = query.value(0).toInt();
  if (g_dimensions <= 0) {
    qWarning() << "Embeddings database has invalid dimensions:" << g_dimensions;
    db.close();
    QSqlDatabase::removeDatabase(kConnectionName);
    return false;
  }

  g_databasePath = fileInfo.absoluteFilePath();
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
  g_dimensions = 0;
}

bool EmbeddingLookup::isOpen() {
  return QSqlDatabase::contains(kConnectionName) &&
         QSqlDatabase::database(kConnectionName).isOpen() && g_dimensions > 0;
}

QString EmbeddingLookup::databasePath() {
  return g_databasePath;
}

int EmbeddingLookup::dimensions() {
  return g_dimensions;
}

QString EmbeddingLookup::normalizeWord(const QString& word) {
  return word.trimmed().toCaseFolded();
}

QSqlDatabase EmbeddingLookup::connection() {
  return QSqlDatabase::database(kConnectionName);
}

EmbeddingLookup::Vector EmbeddingLookup::decodeVector(const QByteArray& blob) {
  Vector vector;
  const int expectedBytes = g_dimensions * static_cast<int>(sizeof(float));
  if (blob.size() != expectedBytes) {
    return vector;
  }

  vector.values.resize(g_dimensions);
  for (int index = 0; index < g_dimensions; ++index) {
    float value = 0.0F;
    std::memcpy(&value, blob.constData() + index * sizeof(float), sizeof(float));
    vector.values[index] = value;
  }
  vector.found = true;
  return vector;
}

EmbeddingLookup::Vector EmbeddingLookup::lookup(const QString& languageCode,
                                                const QString& word) {
  Vector vector;
  if (!isOpen() || languageCode.isEmpty()) {
    return vector;
  }

  const auto normalizedWord = normalizeWord(word);
  if (normalizedWord.isEmpty()) {
    return vector;
  }

  QSqlQuery query(connection());
  query.prepare(
    "SELECT vector FROM word_embeddings "
    "WHERE language_code = :language_code AND word = :word");
  query.bindValue(":language_code", languageCode);
  query.bindValue(":word", normalizedWord);

  if (!query.exec()) {
    qWarning() << "Embedding lookup failed:" << query.lastError();
    return vector;
  }

  if (!query.next()) {
    return vector;
  }

  return decodeVector(query.value("vector").toByteArray());
}

double EmbeddingLookup::cosineSimilarity(const QVector<float>& left, const QVector<float>& right) {
  if (left.isEmpty() || right.isEmpty() || left.size() != right.size()) {
    return 0.0;
  }

  double dot = 0.0;
  double leftNorm = 0.0;
  double rightNorm = 0.0;
  for (int index = 0; index < left.size(); ++index) {
    dot += static_cast<double>(left[index]) * right[index];
    leftNorm += static_cast<double>(left[index]) * left[index];
    rightNorm += static_cast<double>(right[index]) * right[index];
  }

  if (leftNorm <= 0.0 || rightNorm <= 0.0) {
    return 0.0;
  }
  return dot / (std::sqrt(leftNorm) * std::sqrt(rightNorm));
}

}  // namespace lexis

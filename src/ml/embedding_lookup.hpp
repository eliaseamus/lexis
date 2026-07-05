#pragma once

#include <QByteArray>
#include <QSqlDatabase>
#include <QString>
#include <QVector>

#include <optional>

namespace lexis {

class EmbeddingLookup {
 public:
  struct Vector {
    QVector<float> values;
    bool found = false;
  };

  static bool open(const QString& databasePath);
  static void close();
  static bool isOpen();
  static QString databasePath();
  static int dimensions();
  static Vector lookup(const QString& languageCode, const QString& word);
  static double cosineSimilarity(const QVector<float>& left, const QVector<float>& right);

 private:
  static QString normalizeWord(const QString& word);
  static QSqlDatabase connection();
  static Vector decodeVector(const QByteArray& blob);
};

}  // namespace lexis

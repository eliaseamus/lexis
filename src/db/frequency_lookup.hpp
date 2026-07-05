#pragma once

#include <QSqlDatabase>
#include <QString>
#include <QVariantMap>

namespace lexis {

class FrequencyLookup {
 public:
  struct Result {
    int rank = -1;
    double zipf = 0.0;
    QString tier;
    bool found = false;
  };

  static bool open(const QString& databasePath);
  static void close();
  static bool isOpen();
  static QString databasePath();
  static Result lookup(const QString& languageCode, const QString& word);
  static QVariantMap lookupMap(const QString& languageCode, const QString& word);
  static QString tierFromZipf(double zipf);

 private:
  static QString normalizeWord(const QString& word);
  static QSqlDatabase connection();
};

}  // namespace lexis

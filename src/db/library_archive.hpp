#pragma once

#include "schema_migration.hpp"

#include <QJsonObject>
#include <QSet>
#include <QSqlDatabase>
#include <QString>
#include <QUrl>

namespace lexis {

inline constexpr int kArchiveFormatVersion = 1;

class LibraryArchive {
 public:
  static bool exportLanguage(QSqlDatabase& db, const QString& language, const QString& filePath);
  static bool importLanguage(QSqlDatabase& db, const QString& filePath, QString* importedLanguage);
  static bool backupDatabase(const QString& databasePath, const QString& filePath);
  static bool restoreDatabase(const QString& databasePath, const QString& filePath);

 private:
  struct ArchiveItem {
    int id = -1;
    int parentId = kRootParentId;
    bool hasParent = false;
    QString title;
    int type = 0;
    QString creationTime;
    QString modificationTime;
    QString color;
    QString meaning;
    QString dictionarySummary;
    QByteArray image;
    QByteArray audio;
    int frequencyRank = -1;
    QString frequencyTier;
  };

  static QJsonObject buildManifest(const QString& language, int itemCount, int wordCount);
  static QList<ArchiveItem> readItems(QSqlDatabase& db, const QString& language);
  static bool writeArchiveFile(const QString& filePath, const QJsonObject& root);
  static bool readArchiveFile(const QString& filePath, QJsonObject* root);
  static ArchiveItem archiveItemFromJson(const QJsonObject& object);
  static QJsonObject archiveItemToJson(const ArchiveItem& item);
  static QList<ArchiveItem> sortItemsForImport(const QList<ArchiveItem>& items);
};

}  // namespace lexis

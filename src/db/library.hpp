#pragma once

#include <qqml.h>

#include <QObject>
#include <QUrl>

#include <QTemporaryFile>

#include "app_settings.hpp"
#include "library_item.hpp"
#include "library_section.hpp"
#include "pronunciation.hpp"
#include "schema_migration.hpp"
#include "tree_model.hpp"

namespace lexis {

class Library : public QObject {
  Q_OBJECT
  QML_ELEMENT

  Q_PROPERTY(QVector<LibrarySection*> sections READ sections NOTIFY dummy);

 private:
  struct CurrentItem {
    int id = -1;
  };

 private:
  QVector<LibrarySection*> _sections;
  QString _language;
  int _currentParentId = kRootParentId;
  SectionTypeManager _typeManager;
  AppSettings _settings;
  Pronunciation* _pronunciation = nullptr;
  CurrentItem _audioItem;
  QString _databasePath;
  QHash<int, QUrl> _imageUrlCache;
  QHash<int, QTemporaryFile*> _imageFiles;

 public:
  explicit Library(QObject* parent = nullptr);
  Q_INVOKABLE void openDatabase(const QString& name);
  Q_INVOKABLE void addItem(LibraryItem* item);
  Q_INVOKABLE void updateItem(LibraryItem* item, LibrarySectionType oldType);
  Q_INVOKABLE void moveItem(int id, int targetParentId);
  Q_INVOKABLE void deleteItem(int id, LibrarySectionType type);
  Q_INVOKABLE void openLanguage(const QString& language);
  Q_INVOKABLE void openFolder(int parentId);
  Q_INVOKABLE void openRoot();
  Q_INVOKABLE void deleteLanguage(const QString& language);
  Q_INVOKABLE QUrl readAudio(int id);
  Q_INVOKABLE void updateMeaning(int id, const QString& meaning);
  Q_INVOKABLE TreeModel* getStructure();
  Q_INVOKABLE bool exportLanguage(const QString& language, const QUrl& fileUrl);
  Q_INVOKABLE bool importLanguage(const QUrl& fileUrl);
  Q_INVOKABLE bool backupDatabase(const QUrl& fileUrl);
  Q_INVOKABLE bool restoreDatabase(const QUrl& fileUrl);
  Q_INVOKABLE QString databasePath() const {
    return _databasePath;
  }
  Q_INVOKABLE QStringList registeredLanguages() const;
  Q_INVOKABLE QVariantList search(const QString& query) const;
  Q_INVOKABLE QVariantList findByTitle(const QString& title, int excludeItemId = -1) const;
  Q_INVOKABLE QVariantList duplicateItems() const;
  Q_INVOKABLE QVariantMap languageStatistics() const;
  Q_INVOKABLE QVariantMap itemStatistics(int itemId) const;
  Q_INVOKABLE QString currentLanguage() const {
    return _language;
  }
  Q_INVOKABLE QVariantList ancestorPath(int itemId) const;
  Q_INVOKABLE LibraryItem* getItem(int id);
  Q_INVOKABLE QUrl itemImageUrl(int id);
  Q_INVOKABLE void resolveDuplicateGroup(int keepItemId, const QVariantList& items);
  Q_INVOKABLE void reloadCurrentFolder();
  QVector<LibrarySection*> sections() const {
    return _sections;
  }

 private:
  void invalidateImageCache(int id = -1);
  void closeDatabaseConnection();
  void clearSections();
  LibrarySection* getSection(LibrarySectionType type);
  void populateSections();
  void insertItem(LibraryItem&& item);
  void ensureLanguage(const QString& language);
  QString getTitle(int id) const;
  void requestAudio(const QString& title);
  void updateParentModificationTime(int id);
  void addChildItems(int parentId, TreeItem* parent);
  QVariant parentIdVariant(int parentId) const;
  int parentIdFromVariant(const QVariant& value) const;

 private slots:
  void updateAudio(QByteArray audio);

 signals:
  void dummy();
  void languageImported(const QString& language);
  void databaseRestored();
};

}  // namespace lexis

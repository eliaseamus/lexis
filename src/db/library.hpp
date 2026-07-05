#pragma once

#include <qqml.h>

#include <QObject>
#include <QUrl>

#include <QTemporaryFile>

#include "app_settings.hpp"
#include "dictionary.hpp"
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
  struct PendingAudioRequest {
    int itemId = -1;
    QString title;
  };

  struct PendingDictionaryRequest {
    int itemId = -1;
    QString title;
  };

 private:
  QVector<LibrarySection*> _sections;
  QString _language;
  int _currentParentId = kRootParentId;
  SectionTypeManager _typeManager;
  AppSettings _settings;
  Pronunciation* _pronunciation = nullptr;
  Dictionary* _dictionary = nullptr;
  CurrentItem _audioItem;
  int _pendingAudioItemId = -1;
  bool _audioFetchInProgress = false;
  QVector<PendingAudioRequest> _audioRequestQueue;
  int _pendingDictionaryItemId = -1;
  bool _dictionaryFetchInProgress = false;
  QVector<PendingDictionaryRequest> _dictionaryRequestQueue;
  QString _databasePath;
  QHash<int, QUrl> _imageUrlCache;
  QHash<int, QTemporaryFile*> _imageFiles;
  QHash<int, QUrl> _audioUrlCache;
  QHash<int, QTemporaryFile*> _audioFiles;

 public:
  explicit Library(QObject* parent = nullptr);
  Q_INVOKABLE void openDatabase(const QString& name);
  Q_INVOKABLE void addItem(LibraryItem* item, int parentIdOverride = -1);
  Q_INVOKABLE void updateItem(LibraryItem* item, LibrarySectionType oldType);
  Q_INVOKABLE void moveItem(int id, int targetParentId);
  Q_INVOKABLE void deleteItem(int id, LibrarySectionType type);
  Q_INVOKABLE void openLanguage(const QString& language);
  Q_INVOKABLE void openFolder(int parentId);
  Q_INVOKABLE void openRoot();
  Q_INVOKABLE void deleteLanguage(const QString& language);
  Q_INVOKABLE QUrl readAudio(int id);
  Q_INVOKABLE void refreshAudio(int id);
  Q_INVOKABLE void updateMeaning(int id, const QString& meaning);
  Q_INVOKABLE void updateCachedTranslation(int id, const QString& translation);
  Q_INVOKABLE void storeDictionarySummary(int id, const QString& summary);
  Q_INVOKABLE void prefetchDictionary(const QString& title);
  Q_INVOKABLE QString buildDictionarySummary(const QVector<Definition*>& definitions) const;
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
  Q_INVOKABLE QVariantMap languageStatistics() const;
  Q_INVOKABLE QVariantMap itemStatistics(int itemId) const;
  Q_INVOKABLE QVariantList wordsInScope(int scopeRootId = 0) const;
  Q_INVOKABLE QString currentLanguage() const {
    return _language;
  }
  Q_INVOKABLE int currentParentId() const {
    return _currentParentId;
  }
  Q_INVOKABLE QString parentBreadcrumb(int parentId) const;
  Q_INVOKABLE int itemParentId(int itemId) const;
  Q_INVOKABLE QVariantList suggestSubjectGroups(const QString& wordTitle, const QString& meaning = {},
                                                int excludeItemId = -1,
                                                int currentParentId = -1) const;
  Q_INVOKABLE QVariantList proposeWordGroups(int scopeRootId = -1) const;
  Q_INVOKABLE int createGroupWithWords(const QString& name, const QColor& color,
                                       const QVariantList& wordIds);
  Q_INVOKABLE bool hasSemanticIndex() const;
  Q_INVOKABLE QVariantList ancestorPath(int itemId) const;
  Q_INVOKABLE LibraryItem* getItem(int id);
  Q_INVOKABLE QUrl itemImageUrl(int id);
  QVector<LibrarySection*> sections() const {
    return _sections;
  }

 private:
  void invalidateImageCache(int id = -1);
  void invalidateAudioCache(int id = -1);
  QUrl cacheAudioBlob(int id, const QByteArray& compressedAudio);
  void requestAudioForItem(int id, const QString& title);
  void processAudioQueue();
  void finishAudioFetch(int itemId, const QByteArray& audio);
  void requestDictionarySummaryForItem(int id, const QString& title);
  void processDictionaryQueue();
  void finishDictionaryFetch(int itemId, const QVector<Definition*>& definitions);
  void persistDictionarySummary(int id, const QString& summary);
  void closeDatabaseConnection();
  void clearSections();
  LibrarySection* getSection(LibrarySectionType type);
  void populateSections();
  void backfillMissingFrequencies();
  void insertItem(LibraryItem&& item);
  void ensureLanguage(const QString& language);
  void persistFrequency(int id, int rank, const QString& tier);
  QString getTitle(int id) const;
  void updateParentModificationTime(int id);
  void addChildItems(int parentId, TreeItem* parent);
  QVariant parentIdVariant(int parentId) const;
  int parentIdFromVariant(const QVariant& value) const;

 private slots:
  void updateAudio(QByteArray audio);
  void onDictionaryDefinitionsReady(const QVector<Definition*>& definitions);
  void onDictionaryError();

 signals:
  void dummy();
  void audioReady(int itemId, QUrl url);
  void languageImported(const QString& language);
  void databaseRestored();
};

}  // namespace lexis

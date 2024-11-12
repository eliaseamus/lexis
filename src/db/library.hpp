#pragma once

#include <qqml.h>

#include <QObject>
#include <QUrl>

#include "library_section.hpp"
#include "pronunciation.hpp"
#include "app_settings.hpp"
#include "tree_model.hpp"

namespace lexis {

class Library : public QObject {
 Q_OBJECT
 QML_ELEMENT

 Q_PROPERTY(QVector<LibrarySection*> sections READ sections NOTIFY dummy);

 private:
  // used for async operations on database
  struct CurrentItem {
    QString table;
    int id;
  };

 private:
  QVector<LibrarySection*> _sections;
  QString _table;
  SectionTypeManager _typeManager;
  AppSettings _settings;
  Pronunciation* _pronunciation = nullptr;
  CurrentItem _audioItem;

 public:
  explicit Library(QObject* parent = nullptr);
  Q_INVOKABLE void openDatabase(const QString& name);
  Q_INVOKABLE void addItem(LibraryItem* item);
  Q_INVOKABLE void updateItem(LibraryItem* item, LibrarySectionType oldType);
  Q_INVOKABLE void moveItem(int id, const QString& sourceTable, const QString& targetTable);
  Q_INVOKABLE void deleteItem(int id, LibrarySectionType type);
  Q_INVOKABLE void openTable(const QString& name);
  Q_INVOKABLE void openChildTable(int parentID);
  Q_INVOKABLE void dropTableRecursively(const QString& root);
  Q_INVOKABLE void readAudio(int id);
  Q_INVOKABLE void updateMeaning(int id, const QString& meaning);
  Q_INVOKABLE TreeModel* getStructure();
  QVector<LibrarySection*> sections() const {return _sections;}

 private:
  LibraryItem readItem(int id, const QString& table);
  bool removeItem(int id, LibrarySectionType type, const QString& table);
  bool insertItem(LibraryItem& item, const QString& table);
  void clearSections();
  void createTable();
  void createChildTable(int parentID);
  LibrarySection* getSection(LibrarySectionType type);
  void populateSections();
  void insertItem(LibraryItem&& item);
  int getItemID(const QString& title, const QString& table) const;
  QString getTitle(int id, const QString& table) const;
  QStringList getTablesList() const;
  void dropTable(const QString& name);
  void requestAudio(const QString& title);
  void updateParentModificationTime(const QString& table, int id);
  void renameTableRecursively(const QString& oldName, const QString& parentTable, int parentId);
  bool renameTable(const QString& oldName, const QString& newName);
  bool updateParentInfo(const QString& table, const QString& parentTable, int parentId);
  void addChildItems(const QString& table, TreeItem* parent);

 private slots:
  void updateAudio(QByteArray audio);

 signals:
  void dummy();
};

}


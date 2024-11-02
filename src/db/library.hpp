#pragma once

#include <qqml.h>

#include <QObject>
#include <QUrl>

#include "library_section.hpp"
#include "pronunciation.hpp"

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
    LibrarySectionType type;
  };

 private:
  QVector<LibrarySection*> _sections;
  QString _table;
  SectionTypeManager _typeManager;
  Pronunciation* _pronunciation = nullptr;
  CurrentItem _audioItem;

 public:
  explicit Library(QObject* parent = nullptr);
  Q_INVOKABLE void openDatabase(const QString& name);
  Q_INVOKABLE void addItem(LibraryItem* item);
  Q_INVOKABLE void updateItem(LibraryItem* item, LibrarySectionType oldType);
  Q_INVOKABLE void deleteItem(int id, LibrarySectionType type);
  Q_INVOKABLE void openTable(const QString& name);
  Q_INVOKABLE void openChildTable(int parentID);
  Q_INVOKABLE void dropTableRecursively(const QString& root);
  QVector<LibrarySection*> sections() const {return _sections;}

 private:
  void clearSections();
  void createTable();
  void createChildTable(const QString& parentTable, int parentID);
  LibrarySection* getSection(LibrarySectionType type, bool createIfNotExists = true);
  void populateSections();
  void insertItem(LibraryItem&& item);
  int getItemID(const QString& title) const;
  QString getTitle(int id) const;
  QStringList getTablesList() const;
  void dropTable(const QString& name);
  void updateAudio(const QString& title);
  void updateParentModificationTime(const QString& table, int id);

 private slots:
  void updateAudioItem(QByteArray audio);

 signals:
  void dummy();
};

}


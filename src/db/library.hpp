#pragma once

#include <qqml.h>

#include <QObject>
#include <QUrl>

#include "library_section.hpp"

namespace lexis {

class Library : public QObject {
 Q_OBJECT
 QML_ELEMENT

 Q_PROPERTY(QVector<LibrarySection*> sections READ sections NOTIFY dummy);

 private:
  QVector<LibrarySection*> _sections;
  QString _table;
  SectionTypeManager _typeManager;

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
  Q_INVOKABLE int getItemID(const QString& title) const;
  Q_INVOKABLE LibrarySectionType getItemType(int id) const;

 private:
  void clearSections();
  void createTable();
  void createChildTable(const QString& parentTable, int parentID);
  LibrarySection* getSection(LibrarySectionType type);
  void populateSections();
  void insertItem(LibraryItem&& item, QByteArray&& image);
  QStringList getTablesList() const;
  void dropTable(const QString& name);

 signals:
  void dummy();
};

}


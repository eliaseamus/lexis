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

 public:
  explicit Library(QObject* parent = nullptr);
  Q_INVOKABLE void openDatabase(const QString& name);
  Q_INVOKABLE void addItem(LibraryItem* item);
  Q_INVOKABLE void updateItem(const QString& oldTitle, LibrarySectionType oldType, LibraryItem* item);
  Q_INVOKABLE void deleteItem(LibrarySectionType sectionType, const QString& title);
  Q_INVOKABLE void openTable(const QString& name);
  Q_INVOKABLE void openTable(const QString& parentTable, int parentID);
  Q_INVOKABLE void deleteTable(const QString& name);
  QVector<LibrarySection*> sections() const {return _sections;}
  Q_INVOKABLE int getID(const QString& title) const;

 private:
  void clearSections();
  void createTable();
  void createTable(const QString& parentTable, int parentID);
  LibrarySection* getSection(LibrarySectionType type);
  void populateSections();
  void insertItem(LibraryItem&& item, QByteArray&& image);

 signals:
  void dummy();
};

}


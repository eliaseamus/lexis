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
  QString _language;

 public:
  explicit Library(QObject* parent = nullptr);
  QVector<LibrarySection*> sections() const {return _sections;}
  Q_INVOKABLE void addItem(LibraryItem* item);
  Q_INVOKABLE void updateItem(const QString& title, LibraryItem* item);
  Q_INVOKABLE void deleteItem(LibrarySectionType sectionType, const QString& title);
  Q_INVOKABLE void changeLanguage(const QString& language);
  Q_INVOKABLE void deleteLanguage(const QString& language);

 private:
  void createTable();
  LibrarySection* getSection(LibrarySectionType type);
  void populateSections();
  void updateSections(LibraryItem* item);

 signals:
  void dummy();
};

}


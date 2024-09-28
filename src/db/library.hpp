#pragma once

#include <qqml.h>

#include <QObject>
#include <QUrl>
#include <QtSql/QtSql>

#include "library_section.hpp"

namespace lexis {

class Library : public QObject {
 Q_OBJECT
 QML_ELEMENT

 Q_PROPERTY(QVector<LibrarySection*> sections READ sections NOTIFY dummy);

 private:
  QSqlDatabase _db;
  QVector<LibrarySection*> _sections;

 public:
  explicit Library(QObject* parent = nullptr);
  Q_INVOKABLE void addItem(LibraryItem* item);
  Q_INVOKABLE QVector<QStringList> getItemsOfType(const QString& type) const;
  QVector<LibrarySection*> sections() const {return _sections;}

 private:
  void createTable();

 signals:
  void dummy();
};

}


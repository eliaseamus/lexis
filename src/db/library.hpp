#pragma once

#include <qqml.h>

#include <QObject>
#include <QUrl>
#include <QtSql/QtSql>

#include "library_item.hpp"

namespace lexis {

class Library : public QObject {
 Q_OBJECT
 QML_ELEMENT

 private:
  QSqlDatabase _db;

 public:
  explicit Library(QObject* parent = nullptr);
  Q_INVOKABLE void addItem(LibraryItem* item);
  Q_INVOKABLE QVector<QStringList> getItemsOfType(const QString& type) const;

 private:
  void createTable();
};

}


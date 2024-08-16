#pragma once

#include <QObject>
#include <QUrl>
#include <QtSql/QtSql>

namespace lexis {

struct LibraryItem {
  QString title;
  QString type;
  QUrl imageUrl;
};

class Library : public QObject {
 Q_OBJECT
 
 private:
  QSqlDatabase _db;

 public:
  explicit Library(QObject* parent = nullptr);
};

}


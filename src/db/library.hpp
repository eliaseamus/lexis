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
  QSqlTableModel* _model = nullptr;

 public:
  explicit Library(QObject* parent = nullptr);
  QSqlTableModel* getModel() {return _model;}

 public slots:
  void addItem(const LibraryItem& item);

 private:
  void createTable();
  QByteArray readImage(const QUrl& url);
};

}


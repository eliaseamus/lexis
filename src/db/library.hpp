#pragma once

#include <qqml.h>

#include <QObject>
#include <QUrl>
#include <QtSql/QtSql>

namespace lexis {

class LibraryItem : public QObject {
 Q_OBJECT
 QML_ELEMENT

 Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY dummy);
 Q_PROPERTY(QString type READ type WRITE setType NOTIFY dummy);
 Q_PROPERTY(QString author READ author WRITE setAuthor NOTIFY dummy);
 Q_PROPERTY(int year READ year WRITE setYear NOTIFY dummy);
 Q_PROPERTY(bool bc READ bc WRITE setBc NOTIFY dummy);
 Q_PROPERTY(QUrl imageUrl READ imageUrl WRITE setImageUrl NOTIFY dummy);
 Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY dummy);

 private:
  QString _title;
  QString _type;
  QString _author;
  int _year;
  bool _bc;
  QUrl _imageUrl;
  QColor _color;

 public:
  QString title() const {return _title;}
  QString type() const {return _type;}
  QString author() const {return _author;}
  int year() const {return _year;}
  bool bc() const {return _bc;}
  QUrl imageUrl() const {return _imageUrl;}
  QColor color() const {return _color;}

  void setTitle(const QString& title) {_title = title;}
  void setType(const QString& type) {_type= type;}
  void setAuthor(const QString& author) {_type= author;}
  void setYear(int year) {_year = year;}
  void setBc(int bc) {_bc= bc;}
  void setImageUrl(const QUrl& imageUrl) {_imageUrl = imageUrl;}
  void setColor(const QColor& color) {_color = color;}

 signals:
  void dummy();

};

class Library : public QObject {
 Q_OBJECT
 QML_ELEMENT

 private:
  QSqlDatabase _db;
  QSqlTableModel* _model = nullptr;

 public:
  explicit Library(QObject* parent = nullptr);
  QSqlTableModel* getModel() {return _model;}
  Q_INVOKABLE void addItem(const LibraryItem* item);

 private:
  void createTable();
  void createModel();
  QByteArray readFile(const QUrl& url);
};

}


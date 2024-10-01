#pragma once

#include <qqml.h>

#include <QObject>
#include <QUrl>
#include <QColor>
#include <QTemporaryFile>
#include <QTemporaryDir>

#include "section_type.hpp"

namespace lexis {

class LibraryItem : public QObject {
  Q_OBJECT
  QML_ELEMENT

  Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY dummy);
  Q_PROPERTY(LibrarySectionType type READ type WRITE setType NOTIFY dummy);
  Q_PROPERTY(QString author READ author WRITE setAuthor NOTIFY dummy);
  Q_PROPERTY(int year READ year WRITE setYear NOTIFY dummy);
  Q_PROPERTY(bool bc READ bc WRITE setBc NOTIFY dummy);
  Q_PROPERTY(QUrl imageUrl READ imageUrl WRITE setImageUrl NOTIFY dummy);
  Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY dummy);

  static QTemporaryDir dir;

 private:
  QString _title;
  LibrarySectionType _type;
  QString _author;
  int _year;
  bool _bc;
  QUrl _imageUrl;
  QColor _color;
  QTemporaryFile _image;

 public:
  explicit LibraryItem(QObject* parent = nullptr);
  void init(LibraryItem* item);
  QString title() const {return _title;}
  LibrarySectionType type() const {return _type;}
  QString author() const {return _author;}
  int year() const {return _year;}
  bool bc() const {return _bc;}
  QUrl imageUrl() const {return _imageUrl;}
  QByteArray image() const;
  QColor color() const {return _color;}

  void setTitle(const QString& title) {_title = title;}
  void setType(LibrarySectionType type) {_type = type;}
  void setType(const QString& typeName) {_type = SectionTypeManager::librarySectionType(typeName);}
  void setAuthor(const QString& author) {_author = author;}
  void setYear(int year) {_year = year;}
  void setBc(int bc) {_bc = bc;}
  void setImageUrl(const QUrl& imageUrl) {_imageUrl = imageUrl;}
  void setImage(QByteArray&& data);
  void setColor(const QColor& color) {_color = color;}

 signals:
  void dummy();
};

}

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
  Q_PROPERTY(QDateTime creationTime READ creationTime WRITE setCreationTime NOTIFY dummy);
  Q_PROPERTY(QDateTime modificationTime READ modificationTime WRITE setModificationTime NOTIFY dummy);
  Q_PROPERTY(LibrarySectionType type READ type WRITE setType NOTIFY dummy);
  Q_PROPERTY(QUrl imageUrl READ imageUrl WRITE setImageUrl NOTIFY dummy);
  Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY dummy);

 private:
  QString _title;
  QDateTime _creationTime;
  QDateTime _modificationTime;
  LibrarySectionType _type;
  QUrl _imageUrl;
  QColor _color;
  QTemporaryFile _image;

 public:
  explicit LibraryItem(QObject* parent = nullptr);
  void init(LibraryItem&& item, QByteArray&& image);
  QString title() const {return _title;}
  QDateTime creationTime() const {return _creationTime;}
  QDateTime modificationTime() const {return _modificationTime;}
  LibrarySectionType type() const {return _type;}
  QUrl imageUrl() const {return _imageUrl;}
  QByteArray image() const;
  QColor color() const {return _color;}

  void setTitle(const QString& title) {_title = title;}
  void setCreationTime(const QDateTime& time) {_creationTime = time;}
  void setModificationTime(const QDateTime& time) {_modificationTime = time;}
  void setType(LibrarySectionType type) {_type = type;}
  void setType(const QString& typeName);
  void setImageUrl(const QUrl& imageUrl) {_imageUrl = imageUrl;}
  void setImage(QByteArray&& data);
  void setColor(const QColor& color) {_color = color;}

 private:
  void rename();

 signals:
  void dummy();
};

}

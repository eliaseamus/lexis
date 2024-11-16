#pragma once

#include <qqml.h>

#include <QObject>
#include <QUrl>
#include <QColor>
#include <QTemporaryFile>

#include "section_type.hpp"

namespace lexis {

class LibraryItem : public QObject {
  Q_OBJECT
  QML_ELEMENT

  Q_PROPERTY(int itemID READ id WRITE setID NOTIFY dummy);
  Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY dummy);
  Q_PROPERTY(QDateTime creationTime READ creationTime WRITE setCreationTime NOTIFY dummy);
  Q_PROPERTY(QDateTime modificationTime READ modificationTime WRITE setModificationTime NOTIFY dummy);
  Q_PROPERTY(LibrarySectionType type READ type WRITE setType NOTIFY dummy);
  Q_PROPERTY(QUrl imageUrl READ imageUrl WRITE setImageUrl NOTIFY dummy);
  Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY dummy);
  Q_PROPERTY(QUrl audioUrl READ audioUrl WRITE setAudioUrl NOTIFY dummy);
  Q_PROPERTY(QString meaning READ meaning WRITE setMeaning NOTIFY dummy);

 private:
  int _itemID;
  QString _title;
  QDateTime _creationTime;
  QDateTime _modificationTime;
  LibrarySectionType _type;
  QTemporaryFile _image;
  QUrl _imageUrl;
  QColor _color;
  QTemporaryFile _audio;
  QUrl _audioUrl;
  QString _meaning;

 public:
  explicit LibraryItem(QObject* parent = nullptr);
  LibraryItem(LibraryItem&& item);
  LibraryItem& operator=(LibraryItem&& item);
  int id() const {return _itemID;}
  QString title() const {return _title;}
  QDateTime creationTime() const {return _creationTime;}
  QDateTime modificationTime() const {return _modificationTime;}
  LibrarySectionType type() const {return _type;}
  QUrl imageUrl() const {return _imageUrl;}
  QByteArray image() const;
  QColor color() const {return _color;}
  QUrl audioUrl() const {return _audioUrl;}
  QByteArray audio() const;
  QString meaning() const {return _meaning;}

  void setID(int id) {_itemID = id;}
  void setTitle(const QString& title);
  void setCreationTime(const QDateTime& time) {_creationTime = time;}
  void setModificationTime(const QDateTime& time) {_modificationTime = time;}
  void setType(LibrarySectionType type) {_type = type;}
  void setType(const QString& typeName);
  void setImageUrl(const QUrl& imageUrl) {_imageUrl = imageUrl;}
  void setImage(QByteArray&& data);
  void setColor(const QColor& color) {_color = color;}
  void setAudioUrl(const QUrl& audioUrl) {_audioUrl = audioUrl;}
  void setAudio(QByteArray&& data);
  void setMeaning(const QString& meaning) {_meaning = meaning;}

  void freeAssets();

 private:
  void init(LibraryItem&& item);
  QByteArray readFile(const QString& path) const;
  void writeFile(QTemporaryFile& file, QByteArray&& data);

 signals:
  void dummy();

 private:
  Q_DISABLE_COPY(LibraryItem)
};

}

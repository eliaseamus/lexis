#include "library_item.hpp"

#include <QFile>

namespace lexis {

QString fileTemplate = "/Lexis-XXXXXX";
QTemporaryDir dir(QDir::tempPath() + fileTemplate);

LibraryItem::LibraryItem(QObject* parent) :
  QObject(parent)
{
  _image.setFileTemplate(dir.path() + fileTemplate);
}

void LibraryItem::init(LibraryItem&& item, QByteArray&& image) {
  _itemID = item._itemID;
  _title = std::move(item._title);
  _creationTime = std::move(item._creationTime);
  _modificationTime = std::move(item._modificationTime);
  _type = std::move(item._type);
  _color = std::move(item._color);
  setImage(std::move(image));
}

QByteArray LibraryItem::image() const {
  if (_imageUrl.isEmpty()) {
    return {};
  }

  auto path = _imageUrl.toLocalFile();
  QFile image(path);

  if (!image.open(QIODevice::ReadOnly)) {
    qWarning() << "fail to open image:" << path;
    return {};
  }

  return qCompress(image.readAll());
}

void LibraryItem::setTitle(const QString& title) {
  _title = title;
  if (!_title.isEmpty()) {
    _title[0] = _title[0].toUpper();
  }
}

void LibraryItem::setType(const QString& typeName) {
  static SectionTypeManager typeManager;
  _type = typeManager.librarySectionType(typeName);
}

void LibraryItem::setImage(QByteArray&& data) {
  if (data.isEmpty()) {
    return;
  }

  if (!_image.fileName().isEmpty()) {
    rename(); // in order to QML reload image
  }
  if (!_image.open()) {
    qWarning() << "fail to open temporary file" << _image.fileName();
    return;
  }
  _image.resize(0);
  if (-1 == _image.write(qUncompress(data))) {
    qWarning() << "Failed to overwrite image:" << _image.errorString();
  }
  _image.close();
  _imageUrl = QUrl::fromLocalFile(_image.fileName());
}

void LibraryItem::rename() {
  QString newName;
  {
    QTemporaryFile file;
    file.setFileTemplate(dir.path() + fileTemplate);
    file.open();
    newName = file.fileName();
  }
  _image.rename(newName);
}

}

#include "library_item.hpp"

#include <QFile>

namespace lexis {

QTemporaryDir LibraryItem::dir;

LibraryItem::LibraryItem(QObject* parent) :
  QObject(parent)
{
  _image.setFileTemplate(dir.path() + "/" + "Lexis.XXXXXX");
}

void LibraryItem::init(LibraryItem* item) {
  _title = item->title();
  _type = item->type();
  _author = item->author();
  _year = item->year();
  _bc = item->bc();
  _color = item->color();
  setImage(item->image());
}

QByteArray LibraryItem::image() const {
  if (_imageUrl.isEmpty()) {
      qWarning() << "url is empty";
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

void LibraryItem::setImage(QByteArray&& data) {
  if (!_image.open()) {
    qWarning() << "fail to open temporary file" << _image.fileName();
    return;
  }
  _image.write(qUncompress(data));
  _image.close();
  _imageUrl = QUrl::fromLocalFile(_image.fileName());
}

}

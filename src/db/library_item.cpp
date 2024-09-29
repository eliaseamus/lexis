#include "library_item.hpp"

#include <QFile>

namespace lexis {

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

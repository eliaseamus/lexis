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
      qWarning() << "Failed to open image:" << path;
      return {};
    }

  return qCompress(image.readAll());
}

void LibraryItem::setImage(QByteArray&& data) {
  _image.write(qUncompress(data));
  _imageUrl = _image.fileName();
}

}

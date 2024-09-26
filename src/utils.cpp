#include "utils.hpp"

#include <QFile>
#include <QDebug>

namespace lexis {

QByteArray readFile(const QUrl& url) {
  if (url.isEmpty()) {
    qWarning() << "url is empty";
    return {};
  }

  auto path = url.toLocalFile();
  QFile image(path);

  if (!image.open(QIODevice::ReadOnly)) {
    qWarning() << "Failed to open image:" << path;
    return {};
  }

  return image.readAll();
}

}

#include "utils.hpp"

#include <QTemporaryDir>
#include <QTemporaryFile>

namespace lexis {

QString temporaryFileTemplate() {
  static QString fileTemplate = "/Lexis-XXXXXX";
  static QTemporaryDir dir(QDir::tempPath() + fileTemplate);
  return dir.path() + fileTemplate;
}

bool writeCompressedBlob(QTemporaryFile& file, const QByteArray& compressedBlob) {
  if (compressedBlob.isEmpty()) {
    return false;
  }
  if (!file.open()) {
    qWarning() << "fail to open temporary file" << file.fileName();
    return false;
  }
  file.resize(0);
  if (-1 == file.write(qUncompress(compressedBlob))) {
    qWarning() << "Failed to write blob to file:" << file.errorString();
    return false;
  }
  file.close();
  return true;
}

}  // namespace lexis

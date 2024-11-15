#include "utils.hpp"

#include <QTemporaryDir>

namespace lexis {

QString temporaryFileTemplate() {
  static QString fileTemplate = "/Lexis-XXXXXX";
  static QTemporaryDir dir(QDir::tempPath() + fileTemplate);
  return dir.path() + fileTemplate;
}

}

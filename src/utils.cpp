#include "utils.hpp"

#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QRegularExpression>

namespace lexis {

QString temporaryFileTemplate() {
  static QString fileTemplate = "/Lexis-XXXXXX";
  static QTemporaryDir dir(QDir::tempPath() + fileTemplate);
  return dir.path() + fileTemplate;
}

QString formatDateTimeForDb(const QDateTime& dateTime) {
  return dateTime.toString(Qt::ISODate);
}

QDateTime parseDateTimeFromDb(const QString& value) {
  if (value.isEmpty()) {
    return {};
  }

  QDateTime dateTime = QDateTime::fromString(value, Qt::ISODate);
  if (dateTime.isValid()) {
    return dateTime;
  }

  return QDateTime::fromString(value, Qt::TextDate);
}

QString stripHtmlPlainText(const QString& text) {
  QString plain = text;
  plain.remove(QRegularExpression(QStringLiteral("<[^>]*>")));
  plain.replace(QStringLiteral("&nbsp;"), QStringLiteral(" "));
  return plain.trimmed();
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

#include "file_downloader.hpp"

#include <QNetworkReply>

namespace lexis {

FileDownloader::FileDownloader(QObject* parent) :
  WebService(parent)
{
  _file.open();
}

FileDownloader::FileDownloader(const QString& url, QObject* parent) :
  WebService(parent)
{
  _file.open();
  request(url);
}

QByteArray FileDownloader::downloadedData() {
  return _file.readAll();
}

void FileDownloader::onFinished(QNetworkReply* reply) {
  _file.write(reply->readAll());
  _file.flush();
}

}


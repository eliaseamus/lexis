#pragma once

#include "web_service.hpp"

#include <QTemporaryFile>

namespace lexis {

class FileDownloader : public WebService {
 Q_OBJECT

 private:
  QTemporaryFile _file;
 
 public:
  explicit FileDownloader(QObject* parent = nullptr);
  FileDownloader(const QString& url, QObject* parent = nullptr);
  QString name() const {return _file.fileName();}
  QByteArray downloadedData();

 public slots:
  void onFinished(QNetworkReply* reply) override;

};

}


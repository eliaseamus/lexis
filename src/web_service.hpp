#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace lexis {

class WebService : public QObject {
 Q_OBJECT

 private:
  QNetworkAccessManager* _manager = nullptr;

 public:
  explicit WebService(QObject* parent = nullptr);
  virtual void request(const QString& query);

 public slots:
  virtual void onFinished(QNetworkReply* reply) = 0;

};

}


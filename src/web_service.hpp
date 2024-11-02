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
  virtual void get(const QString& query);
  virtual void get(const QNetworkRequest& request);
  virtual void post(const QNetworkRequest& request, const QByteArray& body);

 public slots:
  virtual void onFinished(QNetworkReply* reply) = 0;

};

}


#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

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

 protected:
  bool hasError(QNetworkReply* reply);

 signals:
  void errorOccured(QString);
};

}  // namespace lexis

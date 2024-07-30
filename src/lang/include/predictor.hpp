#pragma once

#include <QObject>
#include <QNetworkAccessManager>

namespace lexis {

class Predictor : public QObject {
 Q_OBJECT

 private:
  QNetworkAccessManager* _manager = nullptr;
  QString _query;

 public:
  explicit Predictor(QObject* parent = nullptr);
  void requestPredictions(const QString& query);

 signals:
  void predictionsReceived(const QStringList& predictions);

 private slots:
  void replyFinished(QNetworkReply* reply);

};

}


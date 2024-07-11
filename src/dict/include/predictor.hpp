#pragma once

#include <QObject>
#include <QNetworkAccessManager>

namespace lexis {

class Predictor : QObject {
 Q_OBJECT

 private:
  QNetworkAccessManager* _manager = nullptr;

 public:
  explicit Predictor(QObject* parent = nullptr);
  std::vector<std::string> getPrediction(std::string_view query);

 private slots:
  void replyFinished(QNetworkReply* reply);

};

}


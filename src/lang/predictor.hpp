#pragma once

#include <qqml.h>

#include "web_service.hpp"

namespace lexis {

class Predictor : public WebService {
 Q_OBJECT
 QML_ELEMENT

 private:
  QString _query;

 public:
  explicit Predictor(QObject* parent = nullptr) : WebService(parent) {}
  Q_INVOKABLE void request(const QString& query) override;

 signals:
  void predictionsReceived(const QStringList& predictions);

 private slots:
  void onFinished(QNetworkReply* reply) override;

};

}


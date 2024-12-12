#pragma once

#include <qqml.h>

#include "app_settings.hpp"
#include "web_service.hpp"

namespace lexis {

class Predictor : public WebService {
  Q_OBJECT
  QML_ELEMENT

 private:
  QString _query;
  AppSettings _settings;

 public:
  explicit Predictor(QObject* parent = nullptr) : WebService(parent) {}
  Q_INVOKABLE void get(const QString& query) override;
  void setQuery(const QString& query) {
    _query = query;
  }
  void retrievePredictions(const QJsonDocument& doc) const;

 signals:
  void predictionsReceived(const QStringList& predictions) const;

 public slots:
  void onFinished(QNetworkReply* reply) override;
};

}  // namespace lexis

#include "predictor.hpp"

#include <QGuiApplication>
#include <QInputMethod>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "utils.hpp"

namespace lexis {

void Predictor::get(const QString& query) {
  static const auto limit = 10;
  static const auto urlFormat = QString(
                                  "https://predictor.yandex.net/api/v1/ \
                                         predict.json/complete?           \
                                         key=%1&q=%2&lang=%3&limit=%4")
                                  .remove(' ');

  setQuery(query);
  auto url = QString(urlFormat).arg(MAKE_STR(PREDICTOR_API_KEY), QString(query).replace(' ', '+'),
                                    _settings.getCurrentLanguage(), QString::number(limit));
  WebService::get(url);
}

void Predictor::onFinished(QNetworkReply* reply) {
  if (hasError(reply)) {
    return;
  }

  retrievePredictions(QJsonDocument::fromJson(reply->readAll()));
}

void Predictor::retrievePredictions(const QJsonDocument& doc) const {
  QJsonObject root = doc.object();
  QJsonArray values = root["text"].toArray();
  int pos = root["pos"].toInt();
  bool isEndOfWord = root["endOfWord"].toBool();

  QVector<QString> predictions;
  predictions.reserve(values.size());
  for (const auto& value : values) {
    auto prediction = _query;
    if (pos < 0) {
      prediction = prediction.sliced(0, prediction.size() + pos);
    } else if (isEndOfWord && pos > 0) {
      prediction += " ";
    }
    prediction += value.toString();
    predictions.push_back(std::move(prediction));
  }
  emit predictionsReceived(predictions);
}

}  // namespace lexis

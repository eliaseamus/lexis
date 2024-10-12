#include "predictor.hpp"
#include "utils.hpp"

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QInputMethod>
#include <QGuiApplication>

namespace lexis {

void Predictor::request(const QString& query) {
  static const auto limit = 10;
  static const auto urlFormat = QString("https://predictor.yandex.net/api/v1/ \
                                         predict.json/complete?               \
                                         key=%1&q=%2&lang=%3&limit=%4").remove(' ');

  _query = query;
  auto url = QString(urlFormat).arg(MAKE_STR(PREDICTOR_API_KEY),
                                      QString(query).replace(' ', '+'),
                                      _settings.getInputLanguage(),
                                      QString::number(limit));
  WebService::request(url);
}

void Predictor::onFinished(QNetworkReply* reply) {
  static const char space = ' ';
  QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
  QJsonObject root = document.object();
  QJsonArray values = root["text"].toArray();
  bool isEndOfWord = root["endOfWord"].toBool();
  bool isPhrase = _query.contains(space);

  QVector<QString> predictions;
  predictions.reserve(values.size());
  for (const auto& value : values) {
    QString prediction = value.toString();
    if (isPhrase) {
      const QString& startOfQuery = _query.mid(0, _query.lastIndexOf(space) + 1);
      const QString& endOfQuery = _query.split(space).constLast();
      if (prediction.startsWith(endOfQuery)) {
        prediction.prepend(startOfQuery);
      } else if (isEndOfWord) {
        prediction.prepend(_query + space);
      } else {
        prediction.prepend(_query);
      }
    } else if (isEndOfWord && !prediction.startsWith(_query)) {
      prediction.prepend(_query + space);
    }
    predictions.push_back(std::move(prediction));
  }
  emit predictionsReceived(predictions);
}

}


#include "dictionary.hpp"

#include "utils.hpp"

#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <qcontainerfwd.h>

namespace lexis {

Dictionary::Dictionary(QObject* parent) :
  QObject(parent)
{
  _manager = new QNetworkAccessManager(this);
  connect(_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
}

void Dictionary::lookup(const QString& query) {
  static const auto lang = "en-ru";
  static const auto urlFormat = "https://dictionary.yandex.net/api/v1/dicservice.json/lookup?key=%1&lang=%2&text=%3";
  _manager->get(QNetworkRequest(QUrl(QString(urlFormat)
    .arg(MAKE_STR(DICTIONARY_API_KEY), lang, QString(query).replace(' ', '+')))));
}

void Dictionary::replyFinished(QNetworkReply* reply) {
  QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
  QJsonObject root = document.object();
  QJsonArray defValues = root["def"].toArray();

  QVector<Definition> definitions;
  definitions.reserve(defValues.size());
  for (const auto& defValue : defValues) {
    Definition def;
    auto defObject = defValue.toObject();

    def.text = defObject["text"].toString();
    def.partOfSpeech = defObject["pos"].toString();
    def.transcription = defObject["ts"].toString();

    auto translationValues = defObject["tr"].toArray();
    QVector<Translation> translations;
    translations.reserve(translationValues.size());

    for (const auto& translationValue : translationValues) {
      Translation translation;
      auto translationObject = translationValue.toObject();

      translation.text = translationObject["text"].toString();
      if (translationObject.contains("syn")) {
        auto synonymValues = translationObject["syn"].toArray();
        QStringList synonyms;
        synonyms.reserve(synonymValues.size());
        for (const auto& synonymValue : synonymValues) {
          auto synonymObject = synonymValue.toObject();
          synonyms.push_back(synonymObject["text"].toString());
        }
        translation.synonyms = std::move(synonyms);
      }

      if (translationObject.contains("mean")) {
        auto meaningValues = translationObject["mean"].toArray();
        QStringList meanings;
        meanings.reserve(meaningValues.size());
        for (const auto& meaningValue : meaningValues) {
          auto meaningObject = meaningValue.toObject();
          meanings.push_back(meaningObject["text"].toString());
        }
        translation.meanings = std::move(meanings);
      }

      translations.push_back(std::move(translation));
    }

    def.translations = std::move(translations);
    definitions.push_back(std::move(def));
  }

  emit definitionsReady(definitions);
}

}


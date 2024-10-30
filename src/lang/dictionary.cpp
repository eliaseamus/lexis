#include "dictionary.hpp"

#include "utils.hpp"

#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

namespace lexis {

void Dictionary::request(const QString& query) {
  static const auto urlFormat = QString("https://dictionary.yandex.net/api/v1/ \
                                         dicservice.json/lookup?               \
                                         key=%1&lang=%2&text=%3").remove(' ');
  auto lang = QString("%1-%2").arg(_settings.getCurrentLanguage(),
                                   _settings.getInterfaceLanguage());
  WebService::request(QString(urlFormat).arg(MAKE_STR(DICTIONARY_API_KEY),
                                             lang,
                                             QString(query).replace(' ', '+')));
}

void Dictionary::onFinished(QNetworkReply* reply) {
  QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
  QJsonObject root = document.object();
  QJsonArray defValues = root["def"].toArray();

  QVector<Definition*> definitions;
  definitions.reserve(defValues.size());
  for (const auto& defValue : defValues) {
    auto* def = new Definition(this);
    auto defObject = defValue.toObject();

    def->setText(defObject["text"].toString());
    def->setPartOfSpeech(defObject["pos"].toString());
    def->setTranscription(defObject["ts"].toString());

    auto translationValues = defObject["tr"].toArray();
    QVector<Translation*> translations;
    translations.reserve(translationValues.size());

    for (const auto& translationValue : translationValues) {
      auto* translation = new Translation(def);
      auto translationObject = translationValue.toObject();

      translation->setText(translationObject["text"].toString());
      if (translationObject.contains("syn")) {
        auto synonymValues = translationObject["syn"].toArray();
        QStringList synonyms;
        synonyms.reserve(synonymValues.size());
        for (const auto& synonymValue : synonymValues) {
          auto synonymObject = synonymValue.toObject();
          synonyms.emplaceBack(synonymObject["text"].toString());
        }
        translation->setSynonyms(std::move(synonyms));
      }

      if (translationObject.contains("mean")) {
        auto meaningValues = translationObject["mean"].toArray();
        QStringList meanings;
        meanings.reserve(meaningValues.size());
        for (const auto& meaningValue : meaningValues) {
          auto meaningObject = meaningValue.toObject();
          meanings.emplaceBack(meaningObject["text"].toString());
        }
        translation->setMeanings(std::move(meanings));
      }

      translations.emplaceBack(std::move(translation));
    }

    def->setTranslations(std::move(translations));
    definitions.emplaceBack(std::move(def));
  }

  emit definitionsReady(definitions);
}

}


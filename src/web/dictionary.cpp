#include "dictionary.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>

#include "utils.hpp"

namespace lexis {

DictionaryCache::DictionaryCache(qsizetype size, QObject* parent) : QObject(parent), _size(size) {
  _cache.reserve(_size);
}

std::optional<QVector<Definition*>> DictionaryCache::getDefinitions(const QString& query,
                                                                    const QString& language) {
  auto findEntry = [&query, &language](const auto& entry) {
    return entry.query.toLower() == query.toLower() && entry.language == language;
  };
  auto def = std::find_if(_cache.cbegin(), _cache.cend(), findEntry);
  if (def != _cache.end()) {
    _cache.move(std::distance(_cache.cbegin(), def), 0);
    return _cache.first().definitions;
  }
  return std::nullopt;
}

void DictionaryCache::addDefinitions(const QVector<Definition*>& definitions,
                                     const QString& language) {
  if (definitions.empty()) {
    return;
  }

  DictionaryEntry entry;
  entry.query = definitions.first()->text();
  entry.language = language;
  entry.definitions = definitions;
  if (_cache.size() == _size) {
    _cache.pop_back();
  }
  _cache.emplaceFront(std::move(entry));
}

Dictionary::Dictionary(QObject* parent)
    : WebService(parent), _cache(new DictionaryCache(20, this)) {}

void Dictionary::get(const QString& query) {
  static const auto urlFormat = QString(
                                  "https://dictionary.yandex.net/api/v1/ \
                                         dicservice.json/lookup?               \
                                         key=%1&lang=%2&text=%3")
                                  .remove(' ');
  auto language = _settings.getCurrentInterfaceLanguagePair();
  if (auto definitions = _cache->getDefinitions(query, language); definitions != std::nullopt) {
    emit definitionsReady(definitions.value());
    return;
  }

  WebService::get(QString(urlFormat).arg(MAKE_STR(DICTIONARY_API_KEY), language,
                                         QString(query).replace(' ', '+')));
}

void Dictionary::onFinished(QNetworkReply* reply) {
  if (hasError(reply)) {
    return;
  }

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

  _cache->addDefinitions(definitions, _settings.getCurrentInterfaceLanguagePair());
  emit definitionsReady(definitions);
}

}  // namespace lexis

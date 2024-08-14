#include "pronunciation_service.hpp"

#include <unordered_map>

#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

namespace lexis {

void PronunciationService::request(const QString& query) {
  static const auto urlFormat = "https://api.dictionaryapi.dev/api/v2/entries/en/%1";
  WebService::request(QString(urlFormat).arg(QString(query).replace(' ', '+')));
}

void PronunciationService::onFinished(QNetworkReply* reply) {
  QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
  if (!document.isArray()) {
    qDebug() << "no audio was found";
    return;
  }

  QJsonArray values = document.array();
  QVector<Pronunciation> pronunciations;
  auto isAccentAdded = [&pronunciations](Accent accent) {
    return std::find_if(pronunciations.begin(), pronunciations.end(), [accent](const auto& pronunciation) {
      return pronunciation.accent == accent;
    }) != pronunciations.end();
  };
  
  for (const auto& value : values) {
    auto object = value.toObject();
    QJsonArray phonetics = object["phonetics"].toArray();

    if (phonetics.isEmpty()) {
      return;
    }

    for (const auto& entry : phonetics) {
      auto phoneticsObject = entry.toObject();
      auto audioUrl = phoneticsObject["audio"].toString();
      if (!audioUrl.isEmpty()) {
        auto accent = getAccent(audioUrl);
        if (!isAccentAdded(accent)) {
          pronunciations.append({audioUrl, accent});
        }
      }
    }

  }
  emit pronunciationReady(pronunciations);
}

Accent PronunciationService::getAccent(const QString& url) const {
  static std::unordered_map<QString, Accent> accents = {
    {"uk", Accent::kBritish},
    {"gb", Accent::kBritish},
    {"us", Accent::kAmerican},
    {"au", Accent::kAustralian}
  };
  static const auto symbolNum = 2;

  auto accent = url.mid(url.lastIndexOf('-') + 1, symbolNum);
  for (const auto& entry : accents) {
    if (accent == entry.first) {
      return entry.second;
    }
  }

  return Accent::kUnknown;
}

}


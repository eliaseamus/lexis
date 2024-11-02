#include "pronunciation.hpp"

#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include <random>

namespace lexis {

void ElevenLabs::get(const QString& query) {
  static const auto urlFormat = "https://api.dictionaryapi.dev/api/v2/entries/en/%1";
  WebService::get(QString(urlFormat).arg(QString(query).replace(' ', '+')));
}

void ElevenLabs::onFinished(QNetworkReply* reply) {
  QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
  if (!document.isArray()) {
    qDebug() << "no audio was found";
    return;
  }

  QJsonArray values = document.array();

  // emit audioReady();
}

void PlayHT::get(const QString& query) {
  static const auto urlFormat = "https://api.dictionaryapi.dev/api/v2/entries/en/%1";
  WebService::get(QString(urlFormat).arg(QString(query).replace(' ', '+')));
}

void PlayHT::onFinished(QNetworkReply* reply) {
  QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
  if (!document.isArray()) {
    qDebug() << "no audio was found";
    return;
  }

  QJsonArray values = document.array();

  // emit audioReady();
}

Pronunciation::Pronunciation(QObject* parent) :
  QObject(parent),
  _elevenLabs(new ElevenLabs(this)),
  _playHT(new PlayHT(this))
{
  connect(_elevenLabs, &ElevenLabs::audioReady, this, &Pronunciation::sendAudio);
  connect(_playHT, &PlayHT::audioReady, this, &Pronunciation::sendAudio);
}

void Pronunciation::get(const QString& query) {
  auto generator = []() -> int {
    std::default_random_engine dre;
    std::uniform_int_distribution<int> di(1, 10);
    return di(dre);
  };
  static bool isEleven = generator() & 1; // randomly assign service for the first-time use
  if (isEleven) {
    _elevenLabs->get(query);
  } else {
    _playHT->get(query);
  }
  isEleven = !isEleven; // interchange service in use
}

void Pronunciation::sendAudio(const QByteArray& audio) {
  emit audioReady(audio);
}

}


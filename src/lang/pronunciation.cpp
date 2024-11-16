#include "pronunciation.hpp"

#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include <random>
#include <thread>

#include "utils.hpp"

namespace lexis {

void TTSService::textToSpeech(const QString& query) {
  using namespace std::chrono_literals;
  static qsizetype voiceIndex = 0;

  int i = 0;
  const int attempts = 10;
  while (!_voices.size()) {
    qWarning() << "No voices saved, request voices once again";
    requestVoices();
    if (++i == attempts) {
      break;
    }
    std::this_thread::sleep_for(500ms);
  }

  if (!_voices.size()) {
    qWarning() << "No voices found, failed to request audio";
    return;
  }

  requestAudio(query, _voices[voiceIndex]);

  if (++voiceIndex >= _voices.size()) {
    voiceIndex = 0;
  }
}

void TTSService::retrieveAudio(QNetworkReply* reply) {
  emit audioReady(qCompress(reply->readAll()));
}

void TTSService::onFinished(QNetworkReply* reply) {
  if (hasError(reply)) {
    return;
  }
  if (_voices.isEmpty()) {
    retrieveVoices(reply);
  } else {
    retrieveAudio(reply);
  }
}

ElevenLabs::ElevenLabs(QObject* parent) :
  TTSService(parent)
{
  _apiKey = MAKE_STR(ELEVEN_LABS_API_KEY);
}

void ElevenLabs::requestVoices() {
  QNetworkRequest request(QUrl("https://api.elevenlabs.io/v1/voices"));
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  request.setRawHeader("Accept", "application/json");
  request.setRawHeader("xi-api-key", _apiKey.toUtf8());
  WebService::get(request);
}

void ElevenLabs::requestAudio(const QString& query, const QString& voice) {
  QNetworkRequest request(QUrl("https://api.elevenlabs.io/v1/text-to-speech/" + voice));
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  request.setRawHeader("Accept", "audio/mpeg");
  request.setRawHeader("xi-api-key", _apiKey.toUtf8());

  QJsonObject obj;
  obj["text"] = query;
  obj["model_id"] = _model;
  QJsonDocument doc(obj);
  QByteArray body = doc.toJson();

  WebService::post(request, body);
}

void ElevenLabs::retrieveVoices(QNetworkReply* reply) {
  QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
  QJsonObject root = document.object();
  QJsonArray voicesValues = root["voices"].toArray();

  QStringList voices;
  voices.reserve(voicesValues.size());
  for (const auto& voiceValue : voicesValues) {
    auto voiceObject = voiceValue.toObject();
    voices.append(voiceObject["voice_id"].toString());
  }
  setVoices(voices);
}

PlayHT::PlayHT(QObject* parent) :
  TTSService(parent)
{
  _userID = MAKE_STR(PLAY_HT_USER);
  _apiKey = MAKE_STR(PLAY_HT_API_KEY);
}

void PlayHT::requestVoices() {
  QNetworkRequest request(QUrl("https://api.play.ht/api/v2/voices"));
  request.setRawHeader("Accept", "application/json");
  request.setRawHeader("X-USER-ID", _userID.toUtf8());
  request.setRawHeader("AUTHORIZATION", _apiKey.toUtf8());
  WebService::get(request);
}

void PlayHT::requestAudio(const QString& query, const QString& voice) {
  QNetworkRequest request(QUrl("https://api.play.ht/api/v2/tts/stream"));
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  request.setRawHeader("Accept", "audio/mpeg");
  request.setRawHeader("X-USER-ID", _userID.toUtf8());
  request.setRawHeader("AUTHORIZATION", _apiKey.toUtf8());

  QJsonObject obj;
  obj["text"] = query;
  obj["voice_engine"] = _voiceEngine;
  obj["voice"] = voice;
  obj["language"] = getLanguage();
  QJsonDocument doc(obj);
  QByteArray body = doc.toJson();

  WebService::post(request, body);
}

void PlayHT::retrieveVoices(QNetworkReply* reply) {
  QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
  QJsonArray voicesValues = document.array();

  QStringList voices;
  voices.reserve(voicesValues.size());
  for (const auto& voiceValue : voicesValues) {
    auto voiceObject = voiceValue.toObject();
    voices.append(voiceObject["id"].toString());
  }
  setVoices(voices);
}

QString PlayHT::getLanguage() {
  static QHash<QString, QString> languages = {
    {"de", "german"},
    {"en", "english"},
    {"es", "spanish"},
    {"fr", "french"},
    {"it", "italian"},
    {"pl", "polish"},
    {"ru", "russian"},
    {"tr", "turkish"},
    {"uk", "ukranian"}
  };

  auto currentLanguage = _settings.getCurrentLanguage();
  if (!languages.contains(currentLanguage)) {
    qWarning() << currentLanguage << "not found";
    return "english";
  }

  return languages[currentLanguage];
}

Pronunciation::Pronunciation(QObject* parent) :
  QObject(parent),
  _elevenLabs(new ElevenLabs(this)),
  _playHT(new PlayHT(this))
{
  _elevenLabs->requestVoices();
  _playHT->requestVoices();
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
    _elevenLabs->textToSpeech(query);
  } else {
    _playHT->textToSpeech(query);
  }
  isEleven = !isEleven; // interchange service in use
}

void Pronunciation::sendAudio(const QByteArray& audio) {
  emit audioReady(audio);
}

}


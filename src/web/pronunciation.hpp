#pragma once

#include "app_settings.hpp"
#include "web_service.hpp"

namespace lexis {

class TTSService : public WebService {
  Q_OBJECT

 private:
  QStringList _voices;

 public:
  explicit TTSService(QObject* parent = nullptr) : WebService(parent) {}
  void textToSpeech(const QString& query);
  virtual void requestVoices() = 0;

 protected:
  virtual void requestAudio(const QString& query, const QString& voice) = 0;
  virtual void retrieveVoices(QNetworkReply* reply) = 0;
  virtual void retrieveAudio(QNetworkReply* reply);

 public slots:
  void onFinished(QNetworkReply* reply) override;

 protected:
  void setVoices(const QStringList& voices) {
    _voices = voices;
  }

 signals:
  void audioReady(const QByteArray& audio);
};

class ElevenLabs : public TTSService {
  Q_OBJECT

 private:
  QString _apiKey;
  const QString _model = "eleven_multilingual_v2";

 public:
  explicit ElevenLabs(QObject* parent = nullptr);
  void requestVoices() override;

 protected:
  void requestAudio(const QString& query, const QString& voice) override;
  void retrieveVoices(QNetworkReply* reply) override;
};

class PlayHT : public TTSService {
  Q_OBJECT

 private:
  QString _userID;
  QString _apiKey;
  const QString _voiceEngine = "Play3.0-mini";
  AppSettings _settings;

 public:
  explicit PlayHT(QObject* parent = nullptr);
  void requestVoices() override;

 protected:
  void requestAudio(const QString& query, const QString& voice) override;
  void retrieveVoices(QNetworkReply* reply) override;

 private:
  QString getLanguage();
};

class Pronunciation : public QObject {
  Q_OBJECT

 private:
  ElevenLabs* _elevenLabs = nullptr;
  PlayHT* _playHT = nullptr;

 public:
  explicit Pronunciation(QObject* parent = nullptr);
  void get(const QString& query);

 private:
  void sendAudio(const QByteArray& audio);

 signals:
  void audioReady(const QByteArray& audio);
};

}  // namespace lexis

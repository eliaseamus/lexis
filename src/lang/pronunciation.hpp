#pragma once

#include <qqml.h>

#include "web_service.hpp"

namespace lexis {

class ElevenLabs : public WebService {
 Q_OBJECT

 private:
  QString _apiKey;
  QStringList _voices;
  const QString _model = "eleven_multilingual_v2";

 public:
  explicit ElevenLabs(QObject* parent = nullptr);
  void textToSpeech(const QString& query);

 public slots:
  void onFinished(QNetworkReply* reply) override;

 private:
  void requestVoices();
  void requestAudio(const QString& query, const QString& voice);
  void retrieveVoices(QNetworkReply* reply);
  void retrieveAudio(QNetworkReply* reply);

 signals:
   void audioReady(const QByteArray& audio);
};

class PlayHT : public WebService {
 Q_OBJECT

 public:
  explicit PlayHT(QObject* parent = nullptr) : WebService(parent) {}
  void textToSpeech(const QString& query);

 public slots:
  void onFinished(QNetworkReply* reply) override;

 signals:
   void audioReady(const QByteArray& audio);
};

class Pronunciation: public QObject {
 Q_OBJECT
 QML_ELEMENT

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

}


#pragma once

#include "web_service.hpp"

namespace lexis {

struct Translation {
  QString text;
  QStringList synonyms;
  QStringList meanings;
};

struct Definition {
  QString text;
  QString partOfSpeech;
  QString transcription;
  QVector<Translation> translations;
};

class Dictionary : public WebService {
 Q_OBJECT

 public:
  explicit Dictionary(QObject* parent = nullptr) : WebService(parent) {}
  void request(const QString& query) override;

 signals:
  void definitionsReady(const QVector<Definition>& definitions);

 public slots:
  void onFinished(QNetworkReply* reply) override;

};

}


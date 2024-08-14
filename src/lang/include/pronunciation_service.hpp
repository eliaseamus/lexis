#pragma once

#include "web_service.hpp"

namespace lexis {

enum class Accent {
  kBritish,
  kAmerican,
  kAustralian,
  kUnknown
};

struct Pronunciation {
  QString url;
  Accent accent;
};

class PronunciationService : public WebService {
 Q_OBJECT
 
 public:
  explicit PronunciationService(QObject* parent = nullptr) : WebService(parent) {}
  void request(const QString& query) override;

 signals:
  void pronunciationReady(const QVector<Pronunciation>& pronunciations);

 public slots:
  void onFinished(QNetworkReply* reply) override;

 private:
  Accent getAccent(const QString& url) const;

};

}


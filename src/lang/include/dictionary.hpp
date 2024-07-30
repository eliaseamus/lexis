#pragma once

#include <QObject>
#include <QNetworkAccessManager>

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

class Dictionary : public QObject {
 Q_OBJECT

 private:
  QNetworkAccessManager* _manager = nullptr;

 public:
  explicit Dictionary(QObject* parent = nullptr);
  void lookup(const QString& query);

 signals:
  void definitionsReady(const QVector<Definition>& definitions);

 public slots:
  void replyFinished(QNetworkReply* reply); 

};

}


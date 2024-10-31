#pragma once

#include <qqml.h>

#include "web_service.hpp"
#include "app_settings.hpp"

namespace lexis {

class Translation : public QObject {
  Q_OBJECT
  QML_ELEMENT

  Q_PROPERTY(QString text READ text WRITE setText NOTIFY dummy);
  Q_PROPERTY(QStringList synonyms READ synonyms WRITE setSynonyms NOTIFY dummy);
  Q_PROPERTY(QStringList meanings READ meanings WRITE setMeanings NOTIFY dummy);

 private:
  QString _text;
  QStringList _synonyms;
  QStringList _meanings;

 public:
  explicit Translation(QObject* parent = nullptr) : QObject(parent) {}
  QString text() const {return _text;}
  QStringList synonyms() const {return _synonyms;}
  QStringList meanings() const {return _meanings;}

  void setText(const QString& text) {_text = text;}
  void setText(QString&& text) {_text = std::move(text);}
  void setSynonyms(const QStringList& synonyms) {_synonyms = synonyms;}
  void setSynonyms(QStringList&& synonyms) {_synonyms = std::move(synonyms);}
  void setMeanings(const QStringList& meanings) {_meanings = meanings;}
  void setMeanings(QStringList&& meanings) {_meanings = std::move(meanings);}

 signals:
  void dummy();
};

class Definition : public QObject {
  Q_OBJECT
  QML_ELEMENT

  Q_PROPERTY(QString text READ text WRITE setText NOTIFY dummy);
  Q_PROPERTY(QString partOfSpeech READ partOfSpeech WRITE setPartOfSpeech NOTIFY dummy);
  Q_PROPERTY(QString transcription READ transcription WRITE setTranscription NOTIFY dummy);
  Q_PROPERTY(QVector<Translation*> translations READ translations WRITE setTranslations NOTIFY dummy);

 private:
  QString _text;
  QString _partOfSpeech;
  QString _transcription;
  QVector<Translation*> _translations;

 public:
  explicit Definition(QObject* parent = nullptr) : QObject(parent) {}
  QString text() const {return _text;}
  QString partOfSpeech() const {return _partOfSpeech;}
  QString transcription() const {return _transcription;}
  QVector<Translation*> translations() const {return _translations;}

  void setText(const QString& text) {_text = text;}
  void setText(QString&& text) {_text = std::move(text);}
  void setPartOfSpeech(const QString& partOfSpeech) {_partOfSpeech = partOfSpeech;}
  void setPartOfSpeech(QString&& partOfSpeech) {_partOfSpeech = std::move(partOfSpeech);}
  void setTranscription(const QString& transcription) {_transcription = transcription;}
  void setTranscription(QString&& transcription) {_transcription = std::move(transcription);}
  void setTranslations(const QVector<Translation*>& translations) {_translations = translations;}
  void setTranslations(QVector<Translation*>&& translations) {_translations = std::move(translations);}

 signals:
  void dummy();
};

class DictionaryCache : public QObject {
 Q_OBJECT

 private:
  struct DictionaryEntry {
    QString query;
    QString language;
    QVector<Definition*> definitions;
  };

 private:
  AppSettings _settings;
  qsizetype _size;
  QVector<DictionaryEntry> _cache;

 public:
  DictionaryCache(qsizetype size, QObject* parent = nullptr);
  std::optional<QVector<Definition*>> getDefinitions(const QString& query);
  void addDefinitions(const QVector<Definition*>& definitions);
};

class Dictionary : public WebService {
 Q_OBJECT
 QML_ELEMENT

 private:
  AppSettings _settings;
  DictionaryCache* _cache = nullptr;

 public:
  explicit Dictionary(QObject* parent = nullptr);
  Q_INVOKABLE void request(const QString& query) override;

 signals:
  void definitionsReady(const QVector<Definition*>& definitions);

 public slots:
  void onFinished(QNetworkReply* reply) override;

};

}


#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

#include "image.hpp"
#include "dictionary.hpp"
#include "pronunciation_player.hpp"
#include "lexis_dialog.hpp"

namespace lexis {

class WordCard : public LexisDialog {
 Q_OBJECT

 private:
  Dictionary* _dict = nullptr;
  PronunciationService* _pronunciationService = nullptr;
  PronunciationPlayer* _pronunciationPlayer = nullptr;
  QLabel* _title = nullptr;
  QLabel* _transcription = nullptr;
  QLabel* _definitions = nullptr;
  Image* _image = nullptr;
  QStringList _imageQueries;

 public:
  explicit WordCard(QWidget* parent = nullptr);
  WordCard(const QString& word, QWidget* parent = nullptr);
  void build(const QString& word);

 public slots:
  void displayDefinitions(const QVector<Definition>& definitions);
  void onImageChosen(const QUrl& url);

 private slots:
  void selectImage();

};

}


#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

#include "image.hpp"
#include "dictionary.hpp"

namespace lexis {

class WordCard : public QWidget {
 Q_OBJECT

 private:
  Dictionary* _dict = nullptr;
  QLabel* _title = nullptr;
  QLabel* _transcription = nullptr;
  QLabel* _definitions = nullptr;
  Image* _image = nullptr;
  QFont _textFont;

 public:
  WordCard(QWidget* parent = nullptr);
  WordCard(const QString& word, QWidget* parent = nullptr);
  void build(const QString& word);

 public slots:
  void displayDefinitions(const QVector<Definition>& definitions);
  void onImageChosen(const QUrl& url);

 private slots:
  void selectImage();

};

}


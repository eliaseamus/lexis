#include "word_card.hpp"
#include <QtWidgets>

namespace lexis {

WordCard::WordCard(QWidget* parent) :
  QWidget(parent)
{
  _dict = new Dictionary(this);
  _title = new QLabel(this);
  auto* layout = new QVBoxLayout;
  layout->addWidget(_title);
  setLayout(layout);

  connect(_dict, SIGNAL(definitionsReady(const QVector<Definition>&)),
           this, SLOT(displayDefinitions(const QVector<Definition>&)));
}

WordCard::WordCard(const QString& word, QWidget* parent) :
  WordCard(parent)
{
  build(word);
}

void WordCard::build(const QString& word) {
  _title->setText(word);
  _dict->lookup(word);
}

void WordCard::displayDefinitions(const QVector<Definition>& definitions) {
  for (const auto& def : definitions) {
    qDebug() << def.text << def.transcription << def.partOfSpeech;
    for (const auto& translation : def.translations) {
      qDebug() << translation.text;
      if (!translation.synonyms.isEmpty()) {
        qDebug() << translation.synonyms;
      }
      if (!translation.meanings.isEmpty()) {
        qDebug() << translation.meanings;
      }
    }
  }
}

}


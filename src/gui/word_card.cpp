#include "word_card.hpp"
#include <QtWidgets>

namespace lexis {

WordCard::WordCard(QWidget* parent) :
  QWidget(parent)
{
  _dict = new Dictionary(this);
  _title = new QLabel(this);
  _transcription = new QLabel(this);
  _image = new QPushButton("?", this);

  QFont titleFont;
  titleFont.setBold(true);
  titleFont.setCapitalization(QFont::Capitalize);
  titleFont.setPointSize(24);
  titleFont.setFamily("Monospace");
  _title->setFont(titleFont);

  _textFont.setPointSize(16);
  _textFont.setFamily("Monospace");
  _transcription->setFont(_textFont);
  _transcription->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  _transcription->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
  _transcription->setStyleSheet("background: #FFF0AE;");

  _briefInfo = new QVBoxLayout;
  auto* topBar = new QHBoxLayout;
  topBar->setSizeConstraint(QLayout::SetMinimumSize);
  topBar->addWidget(_title);
  topBar->addWidget(_transcription);
  _briefInfo->addLayout(topBar);
  _briefInfo->addSpacing(20);
  
  auto* lexisInfo = new QHBoxLayout;
  lexisInfo->addLayout(_briefInfo);
  lexisInfo->addWidget(_image);

  auto* layout = new QVBoxLayout;
  layout->addLayout(lexisInfo);
  setLayout(layout);

  _image->setStyleSheet("background: #F1F1F1");
  _image->setFont(_textFont);

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
  const auto defNum = definitions.size();
  const bool isMultipleDefinitions = defNum > 1;

  int num = 1;
  for (const auto& def : definitions) {
    if (_transcription->text().isEmpty()) {
      _transcription->setText(QString("[%1]").arg(def.transcription));
    }

    auto* posLabel = new QLabel(this);
    posLabel->setFont(_textFont);
    const auto pos = isMultipleDefinitions ? QString("%1. %2").arg(QString::number(num), def.partOfSpeech)
                                           : def.partOfSpeech;
    posLabel->setText(pos);
    _briefInfo->addWidget(posLabel);

    auto* trLabel = new QLabel(this);
    trLabel->setFont(_textFont);
    trLabel->setStyleSheet("margin-left: 10");
  
    QString trText;
    trText.append("<html><ul>");
    for (const auto& translation : def.translations) {
      trText.append("<li>");
      trText.append(translation.text);
      if (!translation.synonyms.isEmpty()) {
        trText.append(QString(" (%1)").arg(translation.synonyms.join(", ")));
      } 
      if (!translation.meanings.isEmpty()) {
        trText.append(QString(": %1").arg(translation.meanings.join(", ")));
      }
      trText.append("</li>");
    }
    trText.append("</ul></html>");
    trLabel->setText(trText);
    _briefInfo->addWidget(trLabel);
    num++;
  }
}

}


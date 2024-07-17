#include "word_card.hpp"
#include <QtWidgets>

namespace lexis {

WordCard::WordCard(QWidget* parent) :
  QWidget(parent)
{
  _dict = new Dictionary(this);
  _title = new QLabel(this);
  _transcription = new QLabel(this);
  _definitions = new QLabel(this);
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

  _definitions->setFont(_textFont);

  auto* dictPage = new QVBoxLayout;
  auto* topBar = new QHBoxLayout;
  topBar->setSizeConstraint(QLayout::SetMinimumSize);
  topBar->addWidget(_title);
  topBar->addWidget(_transcription);
  dictPage->addLayout(topBar);
  dictPage->addSpacing(20);
  dictPage->addWidget(_definitions);
  
  auto* lexisInfo = new QHBoxLayout;
  lexisInfo->addLayout(dictPage);
  lexisInfo->addWidget(_image);

  auto* layout = new QVBoxLayout;
  layout->addLayout(lexisInfo);
  setLayout(layout);

  _image->setStyleSheet("background: #F1F1F1");
  _image->setFont(_textFont);

  connect(_dict, SIGNAL(definitionsReady(const QVector<Definition>&)),
           this, SLOT(displayDefinitions(const QVector<Definition>&)));
  hide();
}

WordCard::WordCard(const QString& word, QWidget* parent) :
  WordCard(parent)
{
  build(word);
}

void WordCard::build(const QString& word) {
  _dict->lookup(word);
}

void WordCard::displayDefinitions(const QVector<Definition>& definitions) {
  const auto defNum = definitions.size();
  const bool isMultipleDefinitions = defNum > 1;

  if (!definitions.isEmpty()) {
    auto def = definitions.front();
    _title->setText(def.text);
    _transcription->setText(QString("[%1]").arg(def.transcription));
  } else {
    _title->setText("");
    _transcription->setText("");
    _definitions->setText("");
  }

  QString defText = "<html>";
  if (isMultipleDefinitions) {
    defText.append("<ol>");
  }

  for (const auto& def : definitions) {
    if (isMultipleDefinitions) {
      defText.append("<li>");
    }
    defText.append(def.partOfSpeech);

    QString trText;
    trText.append("<ul>");
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
    trText.append("</ul>");
    defText.append(std::move(trText));
    if (isMultipleDefinitions) {
      defText.append("</li>");
    }
  }

  if (isMultipleDefinitions) {
    defText.append("</ol>");
  }

  defText.append("</html>");
  _definitions->setText(defText);
  show();
}

}


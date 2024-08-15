#include "word_card.hpp"
#include <QtWidgets>

#include "visualiser.hpp"

namespace lexis {

WordCard::WordCard(QWidget* parent) :
  LexisDialog(parent)
{
  _dict = new Dictionary(this);
  _pronunciationService = new PronunciationService(this);
  _pronunciationPlayer = new PronunciationPlayer(this);
  _title = new QLabel(this);
  _transcription = new QLabel(this);
  _definitions = new QLabel(this);
  _image = new Image("Click to select an image", this);
  _image->addShadow();

  QFont titleFont;
  titleFont.setBold(true);
  titleFont.setCapitalization(QFont::Capitalize);
  titleFont.setPointSize(24);
  titleFont.setFamily("Monospace");
  _title->setFont(titleFont);

  QFont textFont;
  textFont.setPointSize(16);
  textFont.setFamily("Monospace");
  _transcription->setFont(textFont);
  _transcription->setStyleSheet("background: #FFF0AE; padding: 5px 5px 5px 5px; border-radius: 15px; border: 2px solid black");
  _transcription->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);

  _definitions->setFont(textFont);
  _definitions->setWordWrap(true);
  _definitions->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);

  auto* dictPage = new QVBoxLayout;
  auto* topBar = new QHBoxLayout;
  topBar->setSizeConstraint(QLayout::SetMinimumSize);
  topBar->addWidget(_title);
  topBar->addStretch(1);
  topBar->addWidget(_transcription);

  dictPage->setSizeConstraint(QLayout::SetMinimumSize);
  dictPage->addLayout(topBar);
  dictPage->addWidget(_pronunciationPlayer);
  dictPage->addWidget(_definitions);
  dictPage->setAlignment(Qt::AlignTop | Qt::AlignLeft);
 
  auto* imageSide = new QVBoxLayout;
  imageSide->addWidget(_image);

  auto* lexisInfo = new QHBoxLayout;
  lexisInfo->addLayout(dictPage);
  lexisInfo->addLayout(imageSide);

  auto* layout = new QVBoxLayout;
  layout->addLayout(lexisInfo);
  setLayout(layout);

  connect(_dict, SIGNAL(definitionsReady(const QVector<Definition>&)),
           this, SLOT(displayDefinitions(const QVector<Definition>&)));
  connect(_pronunciationService, &PronunciationService::pronunciationReady,
          _pronunciationPlayer, &PronunciationPlayer::addPronunciations);
  connect(_image, &Image::clicked, this, [this]() {_image->darken();});
  connect(_image, &Image::released, this, [this]() {
    _image->brighten();
    selectImage();
  });
  hide();
}

WordCard::WordCard(const QString& word, QWidget* parent) :
  WordCard(parent)
{
  build(word);
}

void WordCard::build(const QString& word) {
  _dict->request(word);
  _pronunciationService->request(word);
}

void WordCard::displayDefinitions(const QVector<Definition>& definitions) {
  const auto defNum = definitions.size();
  const bool isMultipleDefinitions = defNum > 1;

  _imageQueries.clear();

  if (!definitions.isEmpty()) {
    auto def = definitions.front();
    _title->setText(def.text);
    _transcription->setText(QString("[%1]").arg(def.transcription));
    _imageQueries.append(def.text);
  } else {
    _title->setText("");
    _transcription->setText("");
    _definitions->setText("");
    return;
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
      _imageQueries.append(translation.text);
      if (!translation.synonyms.isEmpty()) {
        trText.append(QString(" (%1)").arg(translation.synonyms.join(", ")));
        _imageQueries.append(translation.synonyms);
      } 
      if (!translation.meanings.isEmpty()) {
        trText.append(QString(": %1").arg(translation.meanings.join(", ")));
        _imageQueries.append(translation.meanings);
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

void WordCard::onImageChosen(const QUrl& url) {
  if (url.isEmpty()) {
    qDebug() << "empty url was provided";
    return;
  }
  _image->setImageFromUrl(url);
  _image->setBackgroundColor(Qt::white);
}

void WordCard::selectImage() {
  auto* vis = new Visualiser(this);
  connect(vis, SIGNAL(imageChosen(const QUrl&)), this, SLOT(onImageChosen(const QUrl&)));
  vis->loadImages(_imageQueries);
  vis->exec();
}

}


#include "pronunciation_player.hpp"

#include <QtWidgets>
#include <QAudioOutput>
#include <unordered_map>

#include "icon.hpp"
#include "file_downloader.hpp"

namespace lexis {

PronunciationPlayer::PronunciationPlayer(QWidget* parent) :
  QWidget(parent)
{
  _player = new QMediaPlayer(this);
  _player->setAudioOutput(new QAudioOutput);

  auto layout = new QHBoxLayout;
  layout->addStretch(1);
  setLayout(layout);
}

void PronunciationPlayer::addPronunciations(const QVector<Pronunciation>& pronunciations) {
  static std::unordered_map<Accent, QString> icons = {
    {Accent::kBritish, ":/icons/uk.png"},
    {Accent::kAmerican, ":/icons/us.png"},
    {Accent::kAustralian, ":/icons/au.png"},
    {Accent::kUnknown, ":/icons/audio.png"}
  };
  static const QSize iconSize = {150, 90};
  _pronunciations = pronunciations;
  auto row = layout();
  for (const auto& pronunciation : _pronunciations) {
    auto accent = new Icon(icons[pronunciation.accent], iconSize, this);
    auto audioFile = new FileDownloader(pronunciation.url, this);
    connect(accent, &Icon::clicked, this, [accent]() {accent->darken();});
    connect(accent, &Icon::released, this, [this, audioFile, accent]() {
      _player->setSource(QUrl::fromLocalFile(audioFile->name()));
      _player->play();
      accent->brighten();
    });
    row->addWidget(accent);
  }
  static_cast<QBoxLayout*>(row)->addStretch(1);
}

}


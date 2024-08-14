#include "pronunciation_player.hpp"

#include <QtWidgets>
#include <QAudioOutput>
#include <unordered_map>

#include "icon.hpp"

namespace lexis {

PronunciationPlayer::PronunciationPlayer(QWidget* parent) :
  QWidget(parent)
{
  _player = new QMediaPlayer(this);
  _player->setAudioOutput(new QAudioOutput);
  _audio = new Image(this);
  _audio->setPixmap(":/icons/audio-mute.png");

  auto layout = new QHBoxLayout;
  layout->addWidget(_audio);
  setLayout(layout);
}

void PronunciationPlayer::addPronunciations(const QVector<Pronunciation>& pronunciations) {
  static std::unordered_map<Accent, QString> icons = {
    {Accent::kBritish, ":/icons/uk.png"},
    {Accent::kAmerican, ":/icons/us.png"},
    {Accent::kAustralian, ":/icons/au.png"},
    {Accent::kUnknown, ":/icons/audio.png"},
  };
  _audio->setPixmap(":/icons/audio.png");
  _pronunciations = pronunciations;
  auto row = layout();
  for (const auto& pronunciation : _pronunciations) {
    auto accent = new Icon(icons[pronunciation.accent], this);
    connect(accent, &Icon::released, this, [this, accent, pronunciation]() {
      _player->setSource(QUrl(pronunciation.url));
      _player->play();
    });
    row->addWidget(accent);
  }
}

}


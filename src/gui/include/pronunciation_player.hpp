#pragma once

#include <QWidget>
#include <QtMultimedia/QMediaPlayer>

#include "pronunciation_service.hpp"
#include "image.hpp"

namespace lexis {

class PronunciationPlayer : public QWidget {
 Q_OBJECT

 private:
  QMediaPlayer* _player = nullptr;
  Image* _audio = nullptr;
  QVector<Pronunciation> _pronunciations;

 public:
  explicit PronunciationPlayer(QWidget* parent = nullptr);

 public slots:
  void addPronunciations(const QVector<Pronunciation>& pronunciations);

};

}


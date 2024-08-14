#include "image.hpp"

#include <QtWidgets>

namespace lexis {

Image::Image(QWidget* parent) :
  QWidget(parent)
{
  _label = new QLabel(this);
  _label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  _label->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
  
  auto* layout = new QVBoxLayout;
  layout->setAlignment(Qt::AlignHCenter);
  layout->addWidget(_label);

  setLayout(layout);
}

Image::Image(const QString& startText, QWidget* parent) :
  Image(parent)
{
  setStartText(startText);
}

void Image::setImageFromUrl(const QUrl& url) {
  _url = url;
  setPixmap(_url.toLocalFile());
  setBackgroundColor(Qt::lightGray);
}

void Image::setBackgroundColor(Qt::GlobalColor color) {
  QPalette pal;
  pal.setColor(QPalette::Window, color);
  setAutoFillBackground(true);
  setPalette(pal);
}

void Image::darken() {
  if (!_pixmap.isNull()) {
    auto image = _label->pixmap().toImage();

    for (int x = 0; x < image.width(); ++x) {
      for (int y = 0; y < image.height(); ++y) {
        auto originalColor = image.pixelColor(x, y);
        image.setPixelColor(x, y, originalColor.darker(kGrayScaleFactor));
      }
    }

    _label->setPixmap(QPixmap::fromImage(std::move(image)));
  }
}

void Image::brighten() {
  if (!_pixmap.isNull()) {
    auto image = _label->pixmap().toImage();

    for (int x = 0; x < image.width(); ++x) {
      for (int y = 0; y < image.height(); ++y) {
        auto originalColor = image.pixelColor(x, y);
        image.setPixelColor(x, y, originalColor.lighter(kGrayScaleFactor));
      }
    }

    _label->setPixmap(QPixmap::fromImage(std::move(image)));
  }
}

void Image::setStartText(const QString& text) {
  _label->setText(text);
}

void Image::setPixmap(const QString& name) {
  _pixmap = QPixmap(name);
  qDebug() << name << _label->size();
  _label->setPixmap(_pixmap.scaled(_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void Image::addShadow() {
  if (auto effect = graphicsEffect(); effect != nullptr) {
    return;
  }

  auto* shadow = new QGraphicsDropShadowEffect();
  shadow->setBlurRadius(20);
  shadow->setOffset(10, 10);
  shadow->setColor(Qt::black);
  _label->setGraphicsEffect(shadow);
}

void Image::resizeEvent(QResizeEvent* event) {
  if (_url.isEmpty()) {
    return;
  }
  static auto kScale = 0.99;
  auto size = _label->size();
  auto w = static_cast<int>(size.width() * kScale);
  auto h = static_cast<int>(size.height() * kScale);
  _label->setPixmap(_pixmap.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void Image::mousePressEvent(QMouseEvent* event) {
  emit clicked();
}

void Image::mouseReleaseEvent(QMouseEvent* event) {
  emit released();
}

}


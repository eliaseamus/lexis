#include "image.hpp"

#include <QtWidgets>

namespace lexis {

Image::Image(QWidget* parent) :
  QWidget(parent)
{
  _label = new QLabel(this);
  
  auto* layout = new QVBoxLayout;
  layout->addWidget(_label);
  layout->setAlignment(Qt::AlignHCenter);
  layout->setContentsMargins(0, 0, 0, 0);

  setLayout(layout);
  setContentsMargins(0, 0, 0, 0); 
}

Image::Image(const QString& startText, QWidget* parent) :
  Image(parent)
{
  setStartText(startText);
  setBackgroundColor(Qt::lightGray);
  _label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  _label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void Image::setImageFromUrl(const QUrl& url) {
  _url = url;
  setPixmapFromFile(_url.toLocalFile());
  setBackgroundColor(Qt::lightGray);
}

void Image::setBackgroundColor(Qt::GlobalColor color) {
  QPalette pal;
  pal.setColor(QPalette::Window, color);
  setAutoFillBackground(true);
  setPalette(pal);
}

void Image::darken() {
  if (_pixmap.isNull()) {
    setBackgroundColor(Qt::darkGray);
  } else {
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
  if (_pixmap.isNull()) {
    setBackgroundColor(Qt::lightGray);
  } else {
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

void Image::setPixmapFromFile(const QString& name) {
  setPixmapFromFile(name, _label->size());
}

void Image::setPixmapFromFile(const QString& name, QSize size) {
  _pixmap = QPixmap(name);
  _label->setPixmap(_pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
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


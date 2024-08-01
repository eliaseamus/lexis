#include "image.hpp"

#include <QtWidgets>
#include <qnamespace.h>

namespace lexis {

Image::Image(const QString& startText, QWidget* parent) :
  QWidget(parent)
{
  _label = new QLabel(this);
  _label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  _label->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
  setAcceptDrops(true);
  
  auto* layout = new QVBoxLayout;
  layout->setAlignment(Qt::AlignHCenter);
  layout->addWidget(_label);

  setLayout(layout);
  setBackgroundColor(Qt::GlobalColor::lightGray);
  _label->setText(startText);
  addShadow();
}

void Image::setImageFromUrl(const QUrl& url) {
  _url = url;
  _pixmap = QPixmap(_url.toLocalFile());
  _label->setPixmap(_pixmap.scaled(_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
  setBackgroundColor(Qt::lightGray);
}

void Image::setBackgroundColor(Qt::GlobalColor color) {
  QPalette pal;
  pal.setColor(QPalette::Window, color);
  setAutoFillBackground(true);
  setPalette(pal);
}

void Image::darken() {
  if (_url.isEmpty()) {
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
  if (_url.isEmpty()) {
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

void Image::dragEnterEvent(QDragEnterEvent* event) {
  if (event->mimeData()->hasUrls()) {
    event->acceptProposedAction();
    setBackgroundColor(Qt::darkGray);
    QGuiApplication::setOverrideCursor(QCursor(Qt::DragCopyCursor));
  }
}

void Image::dragLeaveEvent(QDragLeaveEvent* event) {
  event->accept();
  setBackgroundColor(Qt::GlobalColor::lightGray);
  QGuiApplication::restoreOverrideCursor();
}

void Image::dragMoveEvent(QDragMoveEvent* event) {
  event->accept();
}

void Image::dropEvent(QDropEvent* event) {
  if (event->source() == this) {
    event->ignore();
    return;
  }

  if (event->mimeData()->hasUrls()) {
    auto urllist = event->mimeData()->urls();
    for (const auto& url : urllist) {
      if (url.isLocalFile()) {
        setImageFromUrl(url);
        event->acceptProposedAction();
        QGuiApplication::restoreOverrideCursor();
      }
    }
  }
}

void Image::mousePressEvent(QMouseEvent* event) {
  emit clicked();
}

void Image::mouseReleaseEvent(QMouseEvent* event) {
  emit released();
}

}


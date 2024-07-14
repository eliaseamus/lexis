#include "image.hpp"

#include <QtWidgets>

namespace lexis {

Image::Image(QWidget* parent) :
  QWidget(parent)
{
  _label = new QLabel(this);
  _label->setMinimumWidth(400);
  _label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  setAcceptDrops(true);

  auto* layout = new QVBoxLayout;
  layout->setAlignment(Qt::AlignHCenter);
  layout->addWidget(_label);

  auto* actions = new QHBoxLayout;
  auto* ok = new QPushButton("&OK", this);
  auto* cancel = new QPushButton("&Cancel", this);
  actions->addWidget(ok);
  actions->addWidget(cancel);
  layout->addLayout(actions);

  connect(cancel, &QPushButton::clicked, this, [this](){this->clearChosenImage();});

  setLayout(layout);
  setBackgroundColor(Qt::GlobalColor::lightGray);
  clearChosenImage();
  addShadow();
}

void Image::setBackgroundColor(Qt::GlobalColor color) {
  QPalette pal;
  pal.setColor(QPalette::Window, color);
  setAutoFillBackground(true);
  setPalette(pal);
}

void Image::clearChosenImage() {
  _label->setText("Pick an image representing\nthe word and drop it in this area");
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

void Image::dragEnterEvent(QDragEnterEvent* event) {
  if (event->mimeData()->hasUrls()) {
    event->acceptProposedAction();
    setBackgroundColor(Qt::GlobalColor::darkGray);
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
        auto pixmap = QPixmap(url.toLocalFile());
        pixmap = pixmap.scaled(_label->size(), Qt::KeepAspectRatio);
        _label->setPixmap(std::move(pixmap));
        setBackgroundColor(Qt::GlobalColor::lightGray);
        event->acceptProposedAction();
        QGuiApplication::restoreOverrideCursor();
      }
    }
  }
}

}



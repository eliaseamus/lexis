#include "drag_drop_image.hpp"

#include <QtWidgets>

namespace lexis {

DragDropImage::DragDropImage(QWidget* parent) :
  Image(parent)
{
  setAcceptDrops(true);
}

DragDropImage::DragDropImage(const QString& startText, QWidget* parent) :
  Image(startText, parent)
{
  setAcceptDrops(true);
}

void DragDropImage::dragEnterEvent(QDragEnterEvent* event) {
  if (event->mimeData()->hasUrls()) {
    event->acceptProposedAction();
    setBackgroundColor(Qt::darkGray);
    QGuiApplication::setOverrideCursor(QCursor(Qt::DragCopyCursor));
  }
}

void DragDropImage::dragLeaveEvent(QDragLeaveEvent* event) {
  event->accept();
  setBackgroundColor(Qt::GlobalColor::lightGray);
  QGuiApplication::restoreOverrideCursor();
}

void DragDropImage::dragMoveEvent(QDragMoveEvent* event) {
  event->accept();
}

void DragDropImage::dropEvent(QDropEvent* event) {
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

}


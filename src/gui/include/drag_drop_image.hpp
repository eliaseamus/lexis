#pragma once

#include "image.hpp"

namespace lexis {

class DragDropImage : public Image {
 Q_OBJECT

 public:
  DragDropImage(QWidget* parent = nullptr);
  DragDropImage(const QString& startText, QWidget* parent = nullptr);

 protected:
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dragLeaveEvent(QDragLeaveEvent* event) override;
  void dragMoveEvent(QDragMoveEvent* event) override;
  void dropEvent(QDropEvent* event) override;

};

}


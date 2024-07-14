#pragma once

#include <QLabel>
#include <QGraphicsScene>
#include <QEvent>

namespace lexis {

class Image : public QWidget {
 Q_OBJECT

 private:
  QLabel* _label = nullptr;

 public:
  explicit Image(QWidget* parent = nullptr);

 private:
  void addShadow();
  void setBackgroundColor(Qt::GlobalColor color);
  void clearChosenImage();

 protected:
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dragLeaveEvent(QDragLeaveEvent* event) override;
  void dragMoveEvent(QDragMoveEvent* event) override;
  void dropEvent(QDropEvent* event) override;

};

}



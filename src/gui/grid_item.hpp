#pragma once

#include <QFrame>
#include "image.hpp"

namespace lexis {

class GridItem : public QFrame {
 Q_OBJECT

 private:
  Image* _image = nullptr;
  QLabel* _title = nullptr;

 public:
  explicit GridItem(QWidget* parent = nullptr);
  GridItem(const QString& title, const QUrl& imageUrl, QWidget* parent = nullptr);
  void setTitle(const QString& title);
  void setImage(const QUrl& imageUrl);
  void setBackgroundColor(const QColor& color);
};

}

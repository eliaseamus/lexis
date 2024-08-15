#pragma once

#include "image.hpp"

namespace lexis {

class Icon : public Image {
 Q_OBJECT

 public:
  explicit Icon(QWidget* parent = nullptr) : Image(parent) {}
  Icon(const QString& resource, QSize size, QWidget* parent = nullptr);

};

}


#pragma once

#include "image.hpp"

namespace lexis {

class Icon : public Image {
 Q_OBJECT

 public:
  explicit Icon(QWidget* parent = nullptr) : Image(parent) {}
  Icon(const QString& resource, QWidget* parent = nullptr);

 protected:
  void enterEvent(QEnterEvent* event) override;
  void leaveEvent(QEvent* event) override;

};

}


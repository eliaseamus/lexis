#include "icon.hpp"

namespace lexis {

Icon::Icon(const QString& name, QWidget* parent) :
  Icon(parent)
{
  setPixmap(name);
}

void Icon::enterEvent(QEnterEvent* event) {
  darken();
}

void Icon::leaveEvent(QEvent* event) {
  brighten();
}

}


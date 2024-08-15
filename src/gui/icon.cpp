#include "icon.hpp"

namespace lexis {

Icon::Icon(const QString& resource, QWidget* parent) :
  Icon(parent)
{
  setPixmapFromFile(resource);
}

void Icon::enterEvent(QEnterEvent* event) {
  darken();
}

void Icon::leaveEvent(QEvent* event) {
  brighten();
}

}


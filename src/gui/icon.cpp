#include "icon.hpp"

namespace lexis {

Icon::Icon(const QString& resource, QSize size, QWidget* parent) :
  Icon(parent)
{
  setPixmapFromFile(resource, size);
}

}

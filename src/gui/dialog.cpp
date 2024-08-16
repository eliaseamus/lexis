#include "dialog.hpp"

namespace lexis {

Dialog::Dialog(QWidget* parent) :
  QDialog(parent)
{
  resizeWindow();
}

void Dialog::resizeWindow() {
  if (auto* parent = parentWidget(); !parent) {
    qDebug() << "no parent";
//  } else if (auto* grandParent = parent->parentWidget(); !grandParent) {
//    qDebug() << "no grandparent";
  } else {
    static auto kScale = 0.9;
    auto parentRect = parent->geometry();
    move(parentRect.center() - rect().center());
    resize(kScale * parentRect.width(), kScale * parentRect.height());
  }
}

}


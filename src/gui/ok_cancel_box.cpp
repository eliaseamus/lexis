#include "ok_cancel_box.hpp"

#include <QtWidgets>

namespace lexis {

OkCancelButtonBox::OkCancelButtonBox(QWidget* parent) :
  QWidget(parent)
{
  _ok = new QPushButton("&OK", this);
  _cancel = new QPushButton("&Cancel", this);
  
  auto* layout = new QHBoxLayout;
  layout->addWidget(_ok);
  layout->addWidget(_cancel);
  setLayout(layout);

  connect(_ok, &QPushButton::clicked, this, [this](){emit accepted();});
  connect(_cancel, &QPushButton::clicked, this, [this](){emit rejected();});

  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
}

}

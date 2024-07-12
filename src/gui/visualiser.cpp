#include "visualiser.hpp"

#include <QLayout>

#include "utils.hpp"

namespace lexis {

Visualiser::Visualiser(QWidget* parent) :
  QWidget(parent)
{
  _view = new QWebEngineView(this);
  _image = new QLabel(this);

  auto* layout = new QHBoxLayout;
  layout->addWidget(_image);
  layout->addWidget(_view);
  setLayout(layout);
}


void Visualiser::loadImages(const QString& query) {
  static const auto urlFormat = "https://cse.google.com/cse?cx=%1#gsc.tab=1&gsc.q=%2";
  _view->load(QUrl(QString(urlFormat).arg(MAKE_STR(CSE_ID), query)));
}

}


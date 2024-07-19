#include "visualiser.hpp"

#include <QtWidgets>
#include <qboxlayout.h>

#include "utils.hpp"

namespace lexis {

Visualiser::Visualiser(QWidget* parent) :
  QDialog(parent)
{
  _view = new QWebEngineView(this);
  _image = new Image(this);

  connect(_view, SIGNAL(loadFinished(bool)), this, SLOT(onLoadFinished(bool)));
  connect(_image, &Image::chosen, this, [this](const QUrl& url) {
    emit imageChosen(url);
    this->accept();
  });
  connect(_image, &Image::canceled, this, &QDialog::reject);

  auto* layout = new QHBoxLayout;
  auto* splitter = new QSplitter(this);
  auto* frame = new QFrame(this);
  auto* frameLayout = new QVBoxLayout;
  frameLayout->addWidget(_view);
  frame->setLayout(frameLayout);

  splitter->addWidget(frame);
  splitter->addWidget(_image);
  splitter->setSizes(QList<int>({INT_MAX, INT_MAX}));
  layout->addWidget(splitter);
  setLayout(layout);
}

void Visualiser::loadImages(const QString& query) {
  static bool initialized = false;
  static const auto urlFormat = "https://cse.google.com/cse?cx=%1#gsc.tab=1&gsc.q=%2";

  _view->load(QUrl(QString(urlFormat).arg(MAKE_STR(CSE_ID), query)));
  if (!initialized) {
    _view->hide();
    initialized = true;
  }
}

void Visualiser::onLoadFinished(bool ok) {
  if (!ok) {
    qDebug() << "failed to load page";
    return;
  }

  QString code = "                                                  \
    const ids = [\"cse-header\", \"cse-footer\"];                   \
    const classes = [\"gsc-tabsArea\", \"gsc-above-wrapper-area\"]; \
    ids.forEach(removeElementByID);                                 \
    classes.forEach(removeElementByClassName);                      \
                                                                    \
    function removeElementByID(id, index, array) {                  \
      var element = document.getElementById(id);                    \
      element.parentNode.removeChild(element);                      \
    }                                                               \
                                                                    \
    function removeElementByClassName(name, index, array) {         \
      const elements = document.getElementsByClassName(name);       \
      while (elements.length > 0) {                                 \
        elements[0].parentNode.removeChild(elements[0]);            \
      }                                                             \
    }                                                               \
  ";
  _view->page()->runJavaScript(code);
  _view->show();
}

}


#include "visualiser.hpp"

#include <QtWidgets>
#include <qboxlayout.h>

#include "utils.hpp"

namespace lexis {

Visualiser::Visualiser(QWidget* parent) :
  QDialog(parent)
{
  _view = new QWebEngineView(this);
  _image = new Image("Pick an image representing\nthe word and drop it in this area", this);

  connect(_view, SIGNAL(loadFinished(bool)), this, SLOT(onLoadFinished(bool)));

  auto* layout = new QHBoxLayout;
  auto* splitter = new QSplitter(this);
  auto* leftSide = new QFrame(this);
  auto* leftSideLayout = new QVBoxLayout;
  
  auto* rightSide = new QFrame(this);
  auto* rightSideLayout = new QVBoxLayout;
  auto* actions = new QHBoxLayout;
  auto* ok = new QPushButton("&OK", this);
  auto* cancel = new QPushButton("&Cancel", this);
  actions->addWidget(ok);
  actions->addWidget(cancel);
  
  connect(ok, &QPushButton::clicked, this, [this]() {
    emit imageChosen(_image->getUrl());
    this->accept();
  });
  connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
  
  leftSideLayout->addWidget(_view);
  leftSide->setLayout(leftSideLayout);

  rightSideLayout->addWidget(_image);
  rightSideLayout->addLayout(actions);
  rightSide->setLayout(rightSideLayout);

  splitter->addWidget(leftSide);
  splitter->addWidget(rightSide);
  splitter->setSizes(QList<int>({INT_MAX, INT_MAX}));
  layout->addWidget(splitter);
  setLayout(layout);
  resizeWindow();
}

void Visualiser::resizeWindow() {
    if (auto* parent = parentWidget(); !parent) {
      qDebug() << "no parent";
    } else if (auto* grandParent = parent->parentWidget(); !grandParent) {
      qDebug() << "no grandparent";
    } else {
      static auto kScale = 0.9;
      auto parentRect = grandParent->geometry();
      move(parentRect.center() - rect().center());
      resize(kScale * parentRect.width(), kScale * parentRect.height());
    }
}

void Visualiser::loadImages(const QString& query) {
  static const auto urlFormat = "https://cse.google.com/cse?cx=%1#gsc.tab=1&gsc.q=%2";

  _view->load(QUrl(QString(urlFormat).arg(MAKE_STR(CSE_ID), query)));
  _view->hide();
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


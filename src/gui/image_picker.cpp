#include "image_picker.hpp"

#include <QtWidgets>

#include "ok_cancel_box.hpp"
#include "utils.hpp"

namespace lexis {

ImagePicker::ImagePicker(QWidget* parent) :
  Dialog(parent)
{
  _queries = new QComboBox(this);
  _view = new QWebEngineView(this);
  _image = new DragDropImage("Pick an image representing\n\
                              the word and drop it in this area", this);
  _image->addShadow();

  connect(_view, SIGNAL(loadFinished(bool)), this, SLOT(onLoadFinished(bool)));

  auto layout = new QHBoxLayout;
  auto splitter = new QSplitter(this);
  auto leftSide = new QFrame(this);
  auto leftSideLayout = new QVBoxLayout;

  auto rightSide = new QFrame(this);
  auto rightSideLayout = new QVBoxLayout;

  auto okCancel = new OkCancelButtonBox(this);

  connect(_queries, SIGNAL(currentTextChanged(const QString&)),
          this, SLOT(loadImages(const QString&)));
  connect(okCancel, &OkCancelButtonBox::accepted, this, [this]() {
    emit imageChosen(_image->getUrl());
    this->accept();
  });
  connect(okCancel, &OkCancelButtonBox::rejected, this, &QDialog::reject);
  
  _queries->hide();
  _view->hide();
  leftSideLayout->addWidget(_queries);
  leftSideLayout->addWidget(_view);
  leftSide->setLayout(leftSideLayout);

  rightSideLayout->addWidget(_image);
  rightSideLayout->addWidget(okCancel);
  rightSide->setLayout(rightSideLayout);

  splitter->addWidget(leftSide);
  splitter->addWidget(rightSide);
  splitter->setSizes(QList<int>({INT_MAX, INT_MAX}));
  layout->addWidget(splitter);
  setLayout(layout);
}

void ImagePicker::loadImages(const QString& query) {
  static const auto urlFormat = "https://cse.google.com/cse?cx=%1#gsc.tab=1&gsc.q=%2";

  _view->load(QUrl(QString(urlFormat).arg(MAKE_STR(CSE_ID), query)));
}

void ImagePicker::loadImages(const QStringList& queries) {
  if (queries.isEmpty()) {
    qDebug() << "empty queries list";
    return;
  }

  _queries->addItems(queries);
  loadImages(queries.front());
}

void ImagePicker::onLoadFinished(bool ok) {
  if (!ok) {
    qDebug() << "failed to load page";
    return;
  }

  QString code = "                                                  \
    const ids = [\"cse-header\", \"cse-footer\"];                   \
    const classes = [\"gsc-tabsArea\", \"gsc-above-wrapper-area\"]; \
    ids.forEach(removeElementByID);                                 \
    classes.forEach(removeElementByClassName);";
  _view->page()->runJavaScript(code +
                               removeElementByID().code +
                               removeElementByClassName().code);
  _view->show();
  if (!_queries->currentText().isEmpty()) {
    _queries->show();
  }
}

}


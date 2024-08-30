#pragma once

#include <QComboBox>
#include <QSplitter>
#include <QtWebEngineWidgets/QWebEngineView>

#include "dialog.hpp"
#include "drag_drop_image.hpp"

namespace lexis {

class ImagePicker : public Dialog {
 Q_OBJECT

 private:
  QComboBox* _queries = nullptr;
  QWebEngineView* _view = nullptr;
  DragDropImage* _image = nullptr;

 public:
  explicit ImagePicker(QWidget* parent = nullptr);

 public slots:
  void loadImages(const QString& query);
  void loadImages(const QStringList& queries);

 private slots:
  void onLoadFinished(bool ok);

 signals:
  void imageChosen(const QUrl& url);
};

}


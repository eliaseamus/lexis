#pragma once

#include <QWidget>
#include <QSplitter>
#include <QtWebEngineWidgets/QWebEngineView>

#include <image.hpp>

namespace lexis {

class Visualiser : public QWidget {
 Q_OBJECT

 private:
  QWebEngineView* _view = nullptr;
  Image* _image = nullptr;

 public:
  Visualiser(QWidget* aprent = nullptr);

 public slots:
  void loadImages(const QString& query);
  void onLoadFinished(bool ok);
};

}


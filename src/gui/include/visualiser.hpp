#pragma once

#include <QWidget>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QLabel>

namespace lexis {

class Visualiser : public QWidget {
 Q_OBJECT

 private:
  QWebEngineView* _view = nullptr;
  QLabel* _image = nullptr;

 public:
  Visualiser(QWidget* aprent = nullptr);

 public slots:
  void loadImages(const QString& query);

};

}


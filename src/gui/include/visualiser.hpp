#pragma once

#include <QDialog>
#include <QSplitter>
#include <QtWebEngineWidgets/QWebEngineView>

#include <image.hpp>

namespace lexis {

class Visualiser : public QDialog {
 Q_OBJECT

 private:
  QWebEngineView* _view = nullptr;
  Image* _image = nullptr;

 public:
  Visualiser(QWidget* parent = nullptr);

 private:
  void resizeWindow();

 public slots:
  void loadImages(const QString& query);
  void onLoadFinished(bool ok);

 signals:
  void imageChosen(const QUrl& url);
};

}


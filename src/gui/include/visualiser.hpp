#pragma once

#include <QDialog>
#include <QComboBox>
#include <QSplitter>
#include <QtWebEngineWidgets/QWebEngineView>

#include <image.hpp>

namespace lexis {

class Visualiser : public QDialog {
 Q_OBJECT

 private:
  QComboBox* _queries = nullptr;
  QWebEngineView* _view = nullptr;
  Image* _image = nullptr;

 public:
  Visualiser(QWidget* parent = nullptr);

 private:
  void resizeWindow();

 public slots:
  void loadImages(const QString& query);
  void loadImages(const QStringList& queries);

 private slots:
  void onLoadFinished(bool ok);

 signals:
  void imageChosen(const QUrl& url);
};

}


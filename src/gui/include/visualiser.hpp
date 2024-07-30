#pragma once

#include <QComboBox>
#include <QSplitter>
#include <QtWebEngineWidgets/QWebEngineView>

#include "lexis_dialog.hpp"
#include "image.hpp"

namespace lexis {

class Visualiser : public LexisDialog {
 Q_OBJECT

 private:
  QComboBox* _queries = nullptr;
  QWebEngineView* _view = nullptr;
  Image* _image = nullptr;

 public:
  explicit Visualiser(QWidget* parent = nullptr);

 public slots:
  void loadImages(const QString& query);
  void loadImages(const QStringList& queries);

 private slots:
  void onLoadFinished(bool ok);

 signals:
  void imageChosen(const QUrl& url);
};

}


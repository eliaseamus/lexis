#pragma once

#include <QLabel>
#include <QGraphicsScene>
#include <QEvent>

namespace lexis {

class Image : public QWidget {
 Q_OBJECT

 private:
  static constexpr auto kGrayScaleFactor = 200;

 private:
  QLabel* _label = nullptr;
  QUrl _url;
  QPixmap _pixmap;

 public:
  Image(QWidget* parent = nullptr);
  Image(const QString& startText, QWidget* parent = nullptr);
  QUrl getUrl() const {return _url;}
  void setImageFromUrl(const QUrl& url);
  void setBackgroundColor(Qt::GlobalColor color);
  void darken();
  void brighten();
  bool isSet() const {return !_url.isEmpty();}
  void setStartText(const QString& text);
  void setPixmapFromFile(const QString& name);
  void setPixmapFromFile(const QString& name, QSize size);
  void addShadow();
  void expandImage();

 protected:
  void resizeEvent(QResizeEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

 signals:
  void clicked();
  void released();
};

}



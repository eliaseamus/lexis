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

 public:
  Image(const QString& startText, QWidget* parent = nullptr);
  QUrl getUrl() const {return _url;}
  void setImageFromUrl(const QUrl& url);
  void setBackgroundColor(Qt::GlobalColor color);
  void darken();
  void brighten();

 private:
  void addShadow();

 protected:
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dragLeaveEvent(QDragLeaveEvent* event) override;
  void dragMoveEvent(QDragMoveEvent* event) override;
  void dropEvent(QDropEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

 signals:
  void clicked();
  void released();
};

}



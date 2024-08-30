#include "grid_item.hpp"

#include <QtWidgets>

namespace lexis {

GridItem::GridItem(QWidget* parent) :
  QFrame(parent)
{
  _image = new Image(this);
  _title = new QLabel(this);
  _title->setAlignment(Qt::AlignHCenter);

  auto layout = new QVBoxLayout;
  layout->addWidget(_image);
  layout->addWidget(_title);
  layout->setAlignment(Qt::AlignCenter);
  setLayout(layout);
}

GridItem::GridItem(const QString& title, const QUrl& imageUrl, QWidget* parent) :
  GridItem(parent)
{
  setTitle(title);
  setImage(imageUrl);
}

void GridItem::setTitle(const QString& title) {
  _title->setText(title);
}

void GridItem::setImage(const QUrl& imageUrl) {
  _image->setImageFromUrl(imageUrl);
  _image->expandImage();
}

void GridItem::setBackgroundColor(const QColor& color) {
  setStyleSheet(QString("background-color: %1; \
                         border-radius: 30px;  \
                         border: 2px solid %2").arg(color.name(),
                                                    color.darker().name()));
  _image->setStyleSheet("border: 0px;");
  _title->setStyleSheet("border: 0px;");
}

}

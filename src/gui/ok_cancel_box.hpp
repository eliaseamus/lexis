#pragma once

#include <QWidget>
#include <QPushButton>

namespace lexis {

class OkCancelButtonBox : public QWidget {
 Q_OBJECT

 private:
  QPushButton* _ok = nullptr;
  QPushButton* _cancel = nullptr;

 public:
  explicit OkCancelButtonBox(QWidget* parent = nullptr);

 signals:
  void accepted();
  void rejected();

};

}


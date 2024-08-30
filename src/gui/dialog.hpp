#pragma once

#include <QDialog>

namespace lexis {

class Dialog : public QDialog {
 public:
  explicit Dialog(QWidget* parent = nullptr);

 private:
  void resizeWindow();
};

}


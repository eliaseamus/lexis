#pragma once

#include <QDialog>

namespace lexis {

class LexisDialog : public QDialog {
 public:
  explicit LexisDialog(QWidget* parent = nullptr);

 private:
  void resizeWindow();
};

}


#pragma once

#include <QLineEdit>
#include <QComboBox>

#include "lexis_dialog.hpp"
#include "image.hpp"
#include "completer.hpp"

namespace lexis {

class NewLexisDialog : public LexisDialog {
 Q_OBJECT

 private:
  QLineEdit* _title = nullptr;
  Completer* _completer = nullptr;
  Image* _image = nullptr;
  QComboBox* _type = nullptr;

 public:
  explicit NewLexisDialog(QWidget* parent = nullptr);
 
 public slots: 
  void onImageChosen(const QUrl& url);

 private slots:
  void selectImage();
};

}


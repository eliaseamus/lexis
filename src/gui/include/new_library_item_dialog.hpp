#pragma once

#include <QLineEdit>
#include <QComboBox>

#include "dialog.hpp"
#include "image.hpp"
#include "completer.hpp"

namespace lexis {

class NewLibraryItemDialog : public Dialog {
 Q_OBJECT

 private:
  QLineEdit* _title = nullptr;
  Completer* _completer = nullptr;
  Image* _image = nullptr;
  QUrl _imageUrl;
  QComboBox* _type = nullptr;

 public:
  explicit NewLibraryItemDialog(QWidget* parent = nullptr);
 
 public slots: 
  void onImageChosen(const QUrl& url);

 private slots:
  void selectImage();
};

}


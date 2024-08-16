#include "new_library_item_dialog.hpp"

#include "ok_cancel_box.hpp"
#include "image_picker.hpp"

#include <QtWidgets>

namespace lexis {

NewLibraryItemDialog::NewLibraryItemDialog(QWidget* parent) :
  Dialog(parent)
{
  _title = new QLineEdit(this);
  _completer = new Completer(this);
  _image = new Image("Insert title", this);
  _type = new QComboBox(this);

  _title->setPlaceholderText("Title");
  _title->setCompleter(_completer->get());

  _type->addItems(QStringList() << "Book" << "Movie/TV" << "Music" << "Topic");

  _image->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
  _image->addShadow();

  auto* okCancel = new OkCancelButtonBox(this);
  
  auto* layout = new QVBoxLayout;
  layout->addWidget(_title);
  layout->addWidget(_type);
  layout->addWidget(_image);
  layout->addWidget(okCancel);

  setLayout(layout);
  
  connect(_title, &QLineEdit::textEdited, _completer, &Completer::onTextEdited);
  connect(_title, &QLineEdit::textEdited, this, [this](const QString& text) {
    if (_image->isSet()) {
      return;
    }

    if (text.isEmpty()) {
      _image->setStartText("Insert title");
    } else {
      _image->setStartText("Click to select a poster");
    }
  });
  connect(_image, &Image::clicked, this, [this]() {
    if (!_title->text().isEmpty()) {
      _image->darken();
    }
  });
  connect(_image, &Image::released, this, [this]() {
    if (!_title->text().isEmpty()) {
      _image->brighten();
      selectImage();
    }
  });
  connect(okCancel, &OkCancelButtonBox::accepted, this, &QDialog::accept);
  connect(okCancel, &OkCancelButtonBox::rejected, this, &QDialog::reject);
}

void NewLibraryItemDialog::onImageChosen(const QUrl& url) {
  if (url.isEmpty()) {
    qDebug() << "empty url was provided";
    return;
  }
  _imageUrl = url;
  _image->setImageFromUrl(url);
  _image->setBackgroundColor(Qt::white);
}

void NewLibraryItemDialog::selectImage() {
  if (_title->text().isEmpty()) {
    qDebug() << "no title was provided";
    return;
  }

  auto* picker = new ImagePicker(this);
  connect(picker, SIGNAL(imageChosen(const QUrl&)), this, SLOT(onImageChosen(const QUrl&)));
  picker->loadImages(_title->text());
  picker->exec();
}

}

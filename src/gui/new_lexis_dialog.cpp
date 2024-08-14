#include "new_lexis_dialog.hpp"

#include "ok_cancel_box.hpp"
#include "visualiser.hpp"

#include <QtWidgets>

namespace lexis {

NewLexisDialog::NewLexisDialog(QWidget* parent) :
  LexisDialog(parent)
{
  _title = new QLineEdit(this);
  _completer = new Completer(this);
  _image = new Image("Insert title", this);
  _type = new QComboBox(this);

  _title->setPlaceholderText("Title");
  _title->setCompleter(_completer->get());

  _type->addItems(QStringList() << "Book" << "Movie/TV" << "Music" << "Topic");

  _image->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

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

void NewLexisDialog::onImageChosen(const QUrl& url) {
  if (url.isEmpty()) {
    qDebug() << "empty url was provided";
    return;
  }
  _image->setImageFromUrl(url);
  _image->setBackgroundColor(Qt::white);
}

void NewLexisDialog::selectImage() {
  if (_title->text().isEmpty()) {
    qDebug() << "no title was provided";
    return;
  }

  auto* vis = new Visualiser(this);
  connect(vis, SIGNAL(imageChosen(const QUrl&)), this, SLOT(onImageChosen(const QUrl&)));
  vis->loadImages(_title->text());
  vis->exec();
}

}

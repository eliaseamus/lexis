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
  _image = new Image("Click to select a poster", this);
  _type = new QComboBox(this);

  _title->setPlaceholderText("Title");
  _title->setCompleter(_completer->get());

  _type->addItems(QStringList() << "Book" << "Movie/TV" << "Music");

  _image->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

  auto* rightSide = new QVBoxLayout;
  rightSide->addWidget(_title);
  rightSide->addWidget(_type);
  rightSide->setAlignment(Qt::AlignTop);

  auto* lexisFeatures = new QHBoxLayout;
  lexisFeatures->addWidget(_image);
  lexisFeatures->addLayout(rightSide);

  auto* okCancel = new OkCancelButtonBox(this);
  
  auto* layout = new QVBoxLayout;
  layout->addLayout(lexisFeatures);
  layout->addWidget(okCancel);

  setLayout(layout);
  
  connect(_title, &QLineEdit::textEdited, _completer, &Completer::onTextEdited);
  connect(_image, &Image::clicked, this, [this]() {_image->darken();});
  connect(_image, &Image::released, this, [this]() {
    _image->brighten();
    selectImage();
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

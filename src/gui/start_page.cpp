#include "start_page.hpp"
#include "completer.hpp"
#include "new_library_item_dialog.hpp"
#include "word_card.hpp"

#include <QScreen>
#include <QStyleHints>
#include <QtWidgets>

namespace lexis {

StartPage::StartPage(QWidget* parent) :
  QWidget(parent)
{
  _library = new Library(this);
  _libraryView = new QTableView(this);
  _searchLine = new QLineEdit(this);
  _searchButton = new QPushButton("&Search", this);
  _completer = new Completer(this);
  _addItem = new QPushButton("&Add", this);

  _searchLine->setPlaceholderText("Search history..");
  _searchLine->setCompleter(_completer->get());

  _libraryView->setModel(_library->getModel());

  connect(_searchLine, SIGNAL(textEdited(const QString&)), _completer, SLOT(onTextEdited(const QString&)));
  connect(_searchButton, SIGNAL(clicked()), this, SLOT(doSearch()));
  connect(_searchLine, SIGNAL(returnPressed()), this, SLOT(doSearch()));
  connect(_addItem, SIGNAL(clicked()), this, SLOT(addItem()));

  auto* layout = new QVBoxLayout;
  auto* searchBar = new QHBoxLayout;
  searchBar->addWidget(_searchLine);
  searchBar->addWidget(_searchButton);
  
  auto* footer = new QHBoxLayout;
  footer->setAlignment(Qt::AlignRight);
  footer->addWidget(_addItem);

  layout->addLayout(searchBar);
  layout->addWidget(_libraryView);
  layout->addStretch(1);
  layout->addLayout(footer);

  setLayout(layout);
  resizeWindow();
}

void StartPage::resizeWindow() {
  if (!QGuiApplication::styleHints()->showIsFullScreen() && !QGuiApplication::styleHints()->showIsMaximized()) {
    const QRect availableGeometry = this->screen()->availableGeometry();
    this->resize(availableGeometry.width() * 3 / 4, availableGeometry.height() * 3 / 4);
    this->move((availableGeometry.width() - this->width()) / 2,
                (availableGeometry.height() - this->height()) / 2);
  }
}

void StartPage::doSearch() {

}

void StartPage::addItem() {
  auto newLexis = new NewLibraryItemDialog(this);
  connect(newLexis, &NewLibraryItemDialog::newLibraryItemAdded, _library, &Library::addItem);
  newLexis->exec();
  // auto newWord = new WordCard("heed", this);
  // newWord->exec();
}

}


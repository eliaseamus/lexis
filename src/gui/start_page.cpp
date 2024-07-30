#include "start_page.hpp"
#include "completer.hpp"
#include "new_lexis_dialog.hpp"

#include <QScreen>
#include <QStyleHints>
#include <QtWidgets>

namespace lexis {

StartPage::StartPage(QWidget* parent) :
  QWidget(parent)
{
  _searchLine = new QLineEdit(this);
  _searchButton = new QPushButton("&Search", this);
  _completer = new Completer(this);
  _addItem = new QPushButton("&Add", this);

  _searchLine->setPlaceholderText("Search history..");
  _searchLine->setCompleter(_completer->get());

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
  layout->addStretch(1);
  layout->addLayout(footer);

  setLayout(layout);
  resizeWindow();
}

void StartPage::resizeWindow() {
  if (!QGuiApplication::styleHints()->showIsFullScreen() && !QGuiApplication::styleHints()->showIsMaximized()) {
    const QRect availableGeometry = this->screen()->availableGeometry();
    this->resize(availableGeometry.width() / 3, availableGeometry.height() * 2 / 3);
    this->move((availableGeometry.width() - this->width()) / 2,
                (availableGeometry.height() - this->height()) / 2);
  }
}

void StartPage::doSearch() {

}

void StartPage::addItem() {
  auto* newLexis = new NewLexisDialog(this);
  newLexis->exec();
}

}


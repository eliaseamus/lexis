#include "start_page.hpp"
#include "completer.hpp"
#include <QScreen>
#include <QStyleHints>
#include <QtWidgets>

namespace lexis {

StartPage::StartPage(QWidget* parent) :
  QWidget(parent)
{
  _searchLine = new QLineEdit(this);
  _searchButton = new QPushButton("&Search", this);
  _completer = new DictionaryCompleter(this);
  _wordCard = new WordCard(this);

  _searchLine->setPlaceholderText("Search history..");
  _searchLine->setCompleter(_completer->get());

  connect(_searchLine, SIGNAL(textEdited(const QString&)), _completer, SLOT(onTextEdited(const QString&)));
  connect(_searchButton, SIGNAL(clicked()), this, SLOT(doSearch()));
  connect(_searchLine, SIGNAL(returnPressed()), this, SLOT(doSearch()));

  auto layout = new QVBoxLayout;
  auto searchBar = new QHBoxLayout;
  searchBar->addWidget(_searchLine);
  searchBar->addWidget(_searchButton);
  layout->setAlignment(Qt::AlignTop);
  layout->addLayout(searchBar);

  layout->addWidget(_wordCard);
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
  _wordCard->build(_searchLine->text());
//  _visualiser->loadImages(_searchLine->text());
}

}


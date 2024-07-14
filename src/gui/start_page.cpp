#include "start_page.hpp"
#include "completer.hpp"
#include <QScreen>
#include <QStyleHints>
#include <QLayout>
#include <qboxlayout.h>

namespace lexis {

StartPage::StartPage(QWidget* parent) :
  QWidget(parent)
{
  _searchLine = new QLineEdit(this);
  _searchButton = new QPushButton("&Search", this);
  _completer = new DictionaryCompleter(this);
  _visualiser = new Visualiser(this);

  _searchLine->setPlaceholderText("Search history..");
  _searchLine->setCompleter(_completer->get());

  connect(_searchLine, SIGNAL(textEdited(const QString&)), _completer, SLOT(onTextEdited(const QString&)));
  connect(_searchButton, SIGNAL(clicked()), this, SLOT(doSearch()));

  auto layout = new QVBoxLayout;
  auto searchBar = new QHBoxLayout;
  searchBar->addWidget(_searchLine);
  searchBar->addWidget(_searchButton);
  layout->addLayout(searchBar);
  layout->addWidget(_visualiser);
  setLayout(layout);

  resizePage();
}

void StartPage::resizePage() {
  if (!QGuiApplication::styleHints()->showIsFullScreen() && !QGuiApplication::styleHints()->showIsMaximized()) {
    const QRect availableGeometry = this->screen()->availableGeometry();
    this->resize(availableGeometry.width() / 3, availableGeometry.height() * 2 / 3);
    this->move((availableGeometry.width() - this->width()) / 2,
                (availableGeometry.height() - this->height()) / 2);
  }
}

void StartPage::doSearch() {
  _visualiser->loadImages(_searchLine->text());
}

}


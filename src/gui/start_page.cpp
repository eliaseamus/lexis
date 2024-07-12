#include "start_page.hpp"
#include "completer.hpp"
#include <QScreen>
#include <QStyleHints>
#include <QVBoxLayout>

namespace lexis {

StartPage::StartPage(QWidget* parent) :
  QWidget(parent)
{
  _searchLine = new QLineEdit;
  _completer = new DictionaryCompleter(this);
  _searchLine->setPlaceholderText("Search history..");
  _searchLine->setCompleter(_completer->get());

  connect(_searchLine, SIGNAL(textEdited(const QString&)), _completer, SLOT(onTextEdited(const QString&)));

  auto layout = new QVBoxLayout;
  layout->addWidget(_searchLine);
  layout->addStretch(1);
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

}


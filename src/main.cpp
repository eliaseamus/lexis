#include <QApplication>
#include "start_page.hpp"

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  lexis::StartPage startPage;
  startPage.show();

  return app.exec();
}


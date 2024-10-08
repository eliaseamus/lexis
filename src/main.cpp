#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>
#include <QtWebView/QtWebView>

#include "utils.hpp"

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  app.setOrganizationName("Lexis");
  app.setOrganizationDomain("https://github.com/eliaseamus/lexis");
  QtWebView::initialize();
  QQmlApplicationEngine engine;
  auto halt = [](){QCoreApplication::exit(-1);};
  QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                   &app, halt, Qt::QueuedConnection);

  engine.rootContext()->setContextProperty("CSE_ID", MAKE_STR(CSE_ID));
  engine.loadFromModule("QLexis", "Main");

  return app.exec();
}


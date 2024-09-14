#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>
#include <QtWebView/QtWebView>

#include "predictor.hpp"
#include "utils.hpp"

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  QtWebView::initialize();
  QQmlApplicationEngine engine;
  auto halt = [](){QCoreApplication::exit(-1);};
  QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                   &app, halt, Qt::QueuedConnection);

  auto* predictor = new lexis::Predictor(&app);
  engine.rootContext()->setContextProperty("predictor", predictor);
  engine.rootContext()->setContextProperty("CSE_ID", MAKE_STR(CSE_ID));
  engine.loadFromModule("QLexis", "Main");

  return app.exec();
}


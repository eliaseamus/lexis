#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>

#include "predictor.hpp"

int main(int argc, char* argv[]) {
  QGuiApplication app(argc, argv);
  QQmlApplicationEngine engine;
  auto halt = [](){QCoreApplication::exit(-1);};
  QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                   &app, halt, Qt::QueuedConnection);

  auto* predictor = new lexis::Predictor(&app);
  engine.rootContext()->setContextProperty("predictor", predictor);
  engine.loadFromModule("QLexis", "Main");

  return app.exec();
}


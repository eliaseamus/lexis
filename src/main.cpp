#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

int main(int argc, char* argv[]) {
  QGuiApplication app(argc, argv);
  QQmlApplicationEngine engine;
  auto halt = [](){QCoreApplication::exit(-1);};
  QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                   &app, halt, Qt::QueuedConnection);
  QQuickStyle::setStyle("Material");
  engine.loadFromModule("app.lexis", "Main");
  return app.exec();
}


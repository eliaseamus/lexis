#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QtWebView/QtWebView>

#include "app_manager.hpp"
#include "utils.hpp"

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  QtWebView::initialize();
  app.setOrganizationName("Lexis");
  app.setOrganizationDomain("");
  app.setWindowIcon(QIcon(":/QLexis/icons/Lexis.png"));

  QString workingDirectory = qgetenv("LEXIS_WORKDIR");
  if (!workingDirectory.isEmpty()) {
    QDir::setCurrent(workingDirectory);
    qDebug() << "Working directory set to:" << QDir::currentPath();
  }

  QQmlApplicationEngine engine;
  lexis::AppManager appManager(app, engine);
  auto halt = []() {
    QCoreApplication::exit(-1);
  };
  QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, halt,
                   Qt::QueuedConnection);

  engine.rootContext()->setContextProperty("CSE_ID", MAKE_STR(CSE_ID));
  engine.rootContext()->setContextProperty("appManager", &appManager);
  engine.loadFromModule("QLexis", "Main");

  return app.exec();
}

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QtWebView/QtWebView>

#include "app_manager.hpp"
#include "frequency_lookup.hpp"
#include "utils.hpp"

namespace {

bool openFrequencyDatabase() {
  const QStringList candidates = {
    QDir::currentPath() + QStringLiteral("/data/frequency.db"),
    QCoreApplication::applicationDirPath() + QStringLiteral("/data/frequency.db"),
    QCoreApplication::applicationDirPath() + QStringLiteral("/../data/frequency.db"),
  };

  for (const auto& candidate : candidates) {
    if (lexis::FrequencyLookup::open(candidate)) {
      qInfo() << "Loaded frequency database from" << candidate;
      return true;
    }
  }

  qWarning() << "Frequency database not found; word frequency labels will be unavailable";
  return false;
}

}  // namespace

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
  openFrequencyDatabase();
  engine.loadFromModule("QLexis", "Main");

  const auto exitCode = app.exec();
  lexis::FrequencyLookup::close();
  return exitCode;
}

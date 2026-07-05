#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QtWebView/QtWebView>

#include "app_manager.hpp"
#include "embedding_lookup.hpp"
#include "frequency_lookup.hpp"
#include <QDir>
#include <QStandardPaths>
#include "utils.hpp"

namespace {

bool openBundledDatabase(const QString& fileName, bool (*openDatabase)(const QString&),
                         const char* label) {
  const auto appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  const QStringList candidates = {
    QDir::currentPath() + QLatin1Char('/') + fileName,
    appDataDir + QLatin1Char('/') + fileName,
    appDataDir + QStringLiteral("/data/") + fileName,
    QCoreApplication::applicationDirPath() + QStringLiteral("/data/") + fileName,
    QCoreApplication::applicationDirPath() + QStringLiteral("/../data/") + fileName,
  };

  for (const auto& candidate : candidates) {
    if (openDatabase(candidate)) {
      qInfo() << "Loaded" << label << "from" << candidate;
      return true;
    }
  }

  qWarning() << label << "database not found";
  return false;
}

bool openFrequencyDatabase() {
  return openBundledDatabase(QStringLiteral("frequency.db"), lexis::FrequencyLookup::open,
                             "frequency");
}

bool openEmbeddingsDatabase() {
  return openBundledDatabase(QStringLiteral("embeddings.db"), lexis::EmbeddingLookup::open,
                             "embeddings");
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
  openEmbeddingsDatabase();
  engine.loadFromModule("QLexis", "Main");

  const auto exitCode = app.exec();
  lexis::EmbeddingLookup::close();
  lexis::FrequencyLookup::close();
  return exitCode;
}

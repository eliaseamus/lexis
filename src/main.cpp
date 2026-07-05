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

QStringList dataFileCandidates(const QString& fileName) {
  const auto appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  return {
    QDir::currentPath() + QStringLiteral("/") + fileName,
    appDataDir + QStringLiteral("/") + fileName,
    appDataDir + QStringLiteral("/data/") + fileName,
    QCoreApplication::applicationDirPath() + QStringLiteral("/data/") + fileName,
    QCoreApplication::applicationDirPath() + QStringLiteral("/../data/") + fileName,
  };
}

bool openFrequencyDatabase() {
  for (const auto& candidate : dataFileCandidates(QStringLiteral("frequency.db"))) {
    if (lexis::FrequencyLookup::open(candidate)) {
      qInfo() << "Loaded frequency database from" << candidate;
      return true;
    }
  }

  qWarning() << "Frequency database not found; word frequency labels will be unavailable";
  return false;
}

bool openEmbeddingDatabase() {
  for (const auto& candidate : dataFileCandidates(QStringLiteral("embeddings.db"))) {
    if (lexis::EmbeddingLookup::open(candidate)) {
      qInfo() << "Loaded embedding database from" << candidate;
      return true;
    }
  }

  qWarning() << "Embedding database not found; semantic group suggestions will fall back "
                "to lexical matching";
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
  openEmbeddingDatabase();
  engine.loadFromModule("QLexis", "Main");

  const auto exitCode = app.exec();
  lexis::EmbeddingLookup::close();
  lexis::FrequencyLookup::close();
  return exitCode;
}

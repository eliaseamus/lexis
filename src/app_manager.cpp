#include "app_manager.hpp"

namespace lexis {

AppManager::AppManager(QGuiApplication& app, QQmlApplicationEngine& engine, QObject* parent) :
  QObject(parent),
  _app(app),
  _engine(engine),
  _translator(new QTranslator(this))
{
  if (!_translator->load(":i18n/Lexis_ru.qm")) {
    qWarning() << "Failed to load translation file";
  }
}

void AppManager::changeLanguage(const QString& language) {
  if (language == "ru") {
    _app.installTranslator(_translator);
  } else {
    _app.removeTranslator(_translator);
  }
  _engine.retranslate();
}

}

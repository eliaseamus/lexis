#pragma once

#include <QGuiApplication>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QTranslator>

namespace lexis {

class AppManager : public QObject {
  Q_OBJECT

 private:
  QTranslator* _translator = nullptr;
  QGuiApplication& _app;
  QQmlApplicationEngine& _engine;

 public:
  AppManager(QGuiApplication& app, QQmlApplicationEngine& engine, QObject* parent = nullptr);
  Q_INVOKABLE void changeLanguage(const QString& language);
};

}  // namespace lexis

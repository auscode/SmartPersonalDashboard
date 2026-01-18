#include "Backend/metricsservice.h"
#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);
  QQmlApplicationEngine engine;
  // Create MetricsService and expose to QML
  MetricsService metricsService;
  engine.rootContext()->setContextProperty("metrics", &metricsService);
  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
      []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
  engine.loadFromModule("SmartPersonalDashboard", "Main");
  return app.exec();
}

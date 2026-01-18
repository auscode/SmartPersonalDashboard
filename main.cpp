#include "Backend/metricsservice.h"
#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);
  QQmlApplicationEngine engine;

  // Create MetricsService as a pointer and expose to QML
  MetricsService *metricsService = new MetricsService(&app);
  if (metricsService) {
    // Register as singleton for Qt 6 modules
    qmlRegisterSingletonInstance("SmartPersonalDashboard", 1, 0, "Metrics",
                                 metricsService);
    // Fallback for existing QML code using lowercase 'metrics'
    engine.rootContext()->setContextProperty("metrics", metricsService);
  } else {
    qCritical() << "Failed to create MetricsService!";
  }

  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
      []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
  engine.loadFromModule("SmartPersonalDashboard", "Main");
  return app.exec();
}

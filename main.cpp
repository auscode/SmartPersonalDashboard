#include "Backend/metricsservice.h"
#include "Backend/FileWatcherService.h"
#include "Backend/SyscallTracerService.h"
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

  // Create FileWatcherService and expose to QML
  FileWatcherService fileWatcher;
  engine.rootContext()->setContextProperty("fileWatcherService", &fileWatcher);
  fileWatcher.start();

  // Create SyscallTracerService and expose to QML
  SyscallTracerService syscallTracer;
  engine.rootContext()->setContextProperty("syscallTracerService", &syscallTracer);

  // Parse command line trace arguments
  QString traceBinary;
  QStringList traceArgs;
  QStringList cmdArgs = QCoreApplication::arguments();
  for (int i = 1; i < cmdArgs.size(); ++i) {
    if (cmdArgs[i] == "--trace" && i + 1 < cmdArgs.size()) {
      traceBinary = cmdArgs[i + 1];
      i++;
    } else if (cmdArgs[i] == "--trace-args" && i + 1 < cmdArgs.size()) {
      traceArgs = cmdArgs[i + 1].split(' ');
      i++;
    }
  }

  if (!traceBinary.isEmpty()) {
    qDebug() << "Auto-tracing binary from command-line:" << traceBinary << "with args:" << traceArgs;
    syscallTracer.traceBinary(traceBinary, traceArgs);
  }

  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
      []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
  engine.loadFromModule("SmartPersonalDashboard", "Main");
  return app.exec();
}

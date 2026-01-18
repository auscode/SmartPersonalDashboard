#ifndef METRICSSERVICE_H
#define METRICSSERVICE_H

#include <QDateTime>
#include <QMap>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariantList>

class MetricsService : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString currentTime READ currentTime NOTIFY currentTimeChanged)
  Q_PROPERTY(double cpuUsage READ cpuUsage NOTIFY cpuUsageChanged)
  Q_PROPERTY(double gpuTemp READ gpuTemp NOTIFY gpuTempChanged)
  Q_PROPERTY(double memoryUsage READ memoryUsage NOTIFY memoryUsageChanged)
  Q_PROPERTY(double uploadSpeed READ uploadSpeed NOTIFY uploadSpeedChanged)
  Q_PROPERTY(
      double downloadSpeed READ downloadSpeed NOTIFY downloadSpeedChanged)
  Q_PROPERTY(QVariantList drives READ drives NOTIFY drivesChanged)
  Q_PROPERTY(QVariantList hardware READ hardware NOTIFY hardwareChanged)
  Q_PROPERTY(QVariantList processes READ processes NOTIFY processesChanged)
public:
  explicit MetricsService(QObject *parent = nullptr);
  QString currentTime() const;
  double cpuUsage() const;
  double gpuTemp() const;
  double memoryUsage() const;
  double uploadSpeed() const;
  double downloadSpeed() const;
  QVariantList drives() const;
  QVariantList hardware() const;
  QVariantList processes() const;

  Q_INVOKABLE void killProcess(int pid);
  Q_INVOKABLE void suspendProcess(int pid, bool suspend);

signals:
  void currentTimeChanged();
  void cpuUsageChanged();
  void gpuTempChanged();
  void memoryUsageChanged();
  void uploadSpeedChanged();
  void downloadSpeedChanged();
  void drivesChanged();
  void hardwareChanged();
  void processesChanged();
private slots:
  void updateMetrics();

private:
  QTimer m_timer;
  QString m_currentTime;
  double m_cpuUsage = 0.0;
  double m_gpuTemp = 0.0;
  double m_memoryUsage = 0.0;
  double m_uploadSpeed = 0.0;
  double m_downloadSpeed = 0.0;

  // State for CPU calculation
  long long m_prevIdle = 0;
  long long m_prevTotal = 0;

  // State for Network calculation
  long long m_prevRxBytes = 0;
  long long m_prevTxBytes = 0;
  QDateTime m_lastNetworkUpdate;

  QVariantList m_drives;
  QVariantList m_hardware;
  QVariantList m_processes;

  // State for process CPU calculation
  struct ProcessState {
    long long utime;
    long long stime;
    long long total;
  };
  QMap<int, ProcessState> m_prevProcessStates;

  // Helper functions to query system info
  QString fetchCurrentTime();
  double fetchCpuUsage();
  double fetchGpuTemp();
  double fetchMemoryUsage();
  void fetchNetworkSpeeds();
  void fetchDrives();
  void fetchHardware();
  void fetchProcesses();
  double fetchUploadSpeed();
  double fetchDownloadSpeed();
};

#endif // METRICSSERVICE_H

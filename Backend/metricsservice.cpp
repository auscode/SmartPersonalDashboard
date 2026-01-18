#include "metricsservice.h"
#include <QDateTime>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

MetricsService::MetricsService(QObject *parent) : QObject(parent) {
  m_lastNetworkUpdate = QDateTime::currentDateTime();
  connect(&m_timer, &QTimer::timeout, this, &MetricsService::updateMetrics);
  m_timer.start(1000); // update every second
  updateMetrics();
}

QString MetricsService::currentTime() const { return m_currentTime; }
double MetricsService::cpuUsage() const { return m_cpuUsage; }
double MetricsService::gpuTemp() const { return m_gpuTemp; }
double MetricsService::memoryUsage() const { return m_memoryUsage; }
double MetricsService::uploadSpeed() const { return m_uploadSpeed; }
double MetricsService::downloadSpeed() const { return m_downloadSpeed; }

void MetricsService::updateMetrics() {
  m_currentTime = fetchCurrentTime();
  emit currentTimeChanged();

  m_cpuUsage = fetchCpuUsage();
  emit cpuUsageChanged();

  m_memoryUsage = fetchMemoryUsage();
  emit memoryUsageChanged();

  // Network speeds are updated together
  fetchNetworkSpeeds();

  m_gpuTemp = fetchGpuTemp();
  emit gpuTempChanged();
}

QString MetricsService::fetchCurrentTime() {
  return QDateTime::currentDateTime().toString("hh:mm:ss");
}

double MetricsService::fetchCpuUsage() {
  std::ifstream file("/proc/stat");
  std::string line;
  if (std::getline(file, line)) {
    std::istringstream ss(line);
    std::string cpu;
    long long user, nice, system, idle, iowait, irq, softirq, steal;
    ss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >>
        steal;

    long long idleTime = idle + iowait;
    long long totalTime =
        user + nice + system + idle + iowait + irq + softirq + steal;

    long long deltaIdle = idleTime - m_prevIdle;
    long long deltaTotal = totalTime - m_prevTotal;

    m_prevIdle = idleTime;
    m_prevTotal = totalTime;

    if (deltaTotal <= 0)
      return 0.0;
    double usage = (1.0 - (double)deltaIdle / deltaTotal) * 100.0;
    return usage < 0 ? 0 : usage;
  }
  return 0.0;
}

double MetricsService::fetchMemoryUsage() {
  std::ifstream file("/proc/meminfo");
  std::string line;
  long long total = 0, available = 0;
  while (std::getline(file, line)) {
    if (line.find("MemTotal:") == 0) {
      std::sscanf(line.c_str(), "MemTotal: %lld", &total);
    } else if (line.find("MemAvailable:") == 0) {
      std::sscanf(line.c_str(), "MemAvailable: %lld", &available);
    }
  }
  if (total <= 0)
    return 0.0;
  return (double)(total - available) / total * 100.0;
}

void MetricsService::fetchNetworkSpeeds() {
  std::ifstream file("/proc/net/dev");
  std::string line;
  long long totalRx = 0, totalTx = 0;

  // Skip first two lines
  std::getline(file, line);
  std::getline(file, line);

  while (std::getline(file, line)) {
    std::istringstream ss(line);
    std::string iface;
    ss >> iface;
    if (iface == "lo:")
      continue;

    long long rx, tx, dummy;
    ss >> rx; // bytes
    for (int i = 0; i < 7; ++i)
      ss >> dummy;
    ss >> tx; // bytes

    totalRx += rx;
    totalTx += tx;
  }

  QDateTime now = QDateTime::currentDateTime();
  double elapsed = m_lastNetworkUpdate.msecsTo(now) / 1000.0;

  if (elapsed > 0) {
    m_downloadSpeed = (totalRx - m_prevRxBytes) / (1024.0 * 1024.0) / elapsed;
    m_uploadSpeed = (totalTx - m_prevTxBytes) / (1024.0 * 1024.0) / elapsed;

    // Handle overflow or first run
    if (m_downloadSpeed < 0)
      m_downloadSpeed = 0;
    if (m_uploadSpeed < 0)
      m_uploadSpeed = 0;

    emit downloadSpeedChanged();
    emit uploadSpeedChanged();
  }

  m_prevRxBytes = totalRx;
  m_prevTxBytes = totalTx;
  m_lastNetworkUpdate = now;
}

double MetricsService::fetchGpuTemp() {
  // Try common thermal zones
  for (int i = 0; i < 10; ++i) {
    std::string path =
        "/sys/class/thermal/thermal_zone" + std::to_string(i) + "/temp";
    std::ifstream file(path);
    if (file.is_open()) {
      int temp;
      file >> temp;
      return temp / 1000.0;
    }
  }
  return 0.0;
}

// Dummy implementations for compatibility with header if needed
double MetricsService::fetchUploadSpeed() { return m_uploadSpeed; }
double MetricsService::fetchDownloadSpeed() { return m_downloadSpeed; }

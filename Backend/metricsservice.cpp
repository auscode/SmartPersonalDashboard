#include "metricsservice.h"
#include <QDateTime>
#include <QStorageInfo>
#include <QVariantMap>
#include <algorithm>
#include <csignal>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;

MetricsService::MetricsService(QObject *parent) : QObject(parent) {
  m_lastNetworkUpdate = QDateTime::currentDateTime();
  connect(&m_timer, &QTimer::timeout, this, &MetricsService::updateMetrics);
  m_timer.start(2000); // update every 2 seconds for hardware
  updateMetrics();
}

QString MetricsService::currentTime() const { return m_currentTime; }
double MetricsService::cpuUsage() const { return m_cpuUsage; }
double MetricsService::gpuTemp() const { return m_gpuTemp; }
double MetricsService::memoryUsage() const { return m_memoryUsage; }
double MetricsService::uploadSpeed() const { return m_uploadSpeed; }
double MetricsService::downloadSpeed() const { return m_downloadSpeed; }
QVariantList MetricsService::drives() const { return m_drives; }
QVariantList MetricsService::hardware() const { return m_hardware; }
QVariantList MetricsService::processes() const { return m_processes; }

void MetricsService::killProcess(int pid) { ::kill(pid, SIGKILL); }

void MetricsService::suspendProcess(int pid, bool suspend) {
  ::kill(pid, suspend ? SIGSTOP : SIGCONT);
}

void MetricsService::toggleHardware(QString sysfsPath, bool enabled) {
  std::string path = sysfsPath.toStdString();
  if (path.find("/sys/bus/usb/devices") != std::string::npos) {
    std::ofstream file(path + "/authorized");
    if (file.is_open()) {
      file << (enabled ? "1" : "0");
    }
  } else if (path.find("/sys/class/input") != std::string::npos) {
    // For input devices, we try to unbind/bind from driver
    std::string deviceDir = path + "/device";
    std::string driverPath = deviceDir + "/driver";
    if (fs::exists(driverPath)) {
      std::string deviceName = fs::read_symlink(deviceDir).filename().string();
      if (!enabled) {
        std::ofstream unbindFile(driverPath + "/unbind");
        if (unbindFile.is_open())
          unbindFile << deviceName;
      }
    }
  }
}

void MetricsService::updateMetrics() {
  m_currentTime = fetchCurrentTime();
  emit currentTimeChanged();

  m_cpuUsage = fetchCpuUsage();
  emit cpuUsageChanged();

  m_memoryUsage = fetchMemoryUsage();
  emit memoryUsageChanged();

  fetchNetworkSpeeds();

  m_gpuTemp = fetchGpuTemp();
  emit gpuTempChanged();

  fetchDrives();
  fetchHardware();
  fetchProcesses();
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

  std::getline(file, line);
  std::getline(file, line);

  while (std::getline(file, line)) {
    std::istringstream ss(line);
    std::string iface;
    ss >> iface;
    if (iface == "lo:")
      continue;

    long long rx, tx, dummy;
    ss >> rx;
    for (int i = 0; i < 7; ++i)
      ss >> dummy;
    ss >> tx;

    totalRx += rx;
    totalTx += tx;
  }

  QDateTime now = QDateTime::currentDateTime();
  double elapsed = m_lastNetworkUpdate.msecsTo(now) / 1000.0;

  if (elapsed > 0) {
    m_downloadSpeed = (totalRx - m_prevRxBytes) / (1024.0 * 1024.0) / elapsed;
    m_uploadSpeed = (totalTx - m_prevTxBytes) / (1024.0 * 1024.0) / elapsed;

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

void MetricsService::fetchDrives() {
  QVariantList driveList;
  for (const QStorageInfo &storage : QStorageInfo::mountedVolumes()) {
    if (storage.isValid() && storage.isReady()) {
      if (storage.isReadOnly())
        continue;

      QVariantMap drive;
      drive["name"] = storage.displayName();
      drive["root"] = storage.rootPath();
      drive["total"] = storage.bytesTotal() / (1024.0 * 1024.0 * 1024.0); // GB
      drive["available"] =
          storage.bytesAvailable() / (1024.0 * 1024.0 * 1024.0); // GB
      drive["usage"] =
          (1.0 - (double)storage.bytesAvailable() / storage.bytesTotal()) *
          100.0;
      driveList.append(drive);
    }
  }
  if (m_drives != driveList) {
    m_drives = driveList;
    emit drivesChanged();
  }
}

void MetricsService::fetchHardware() {
  QVariantList hwList;
  // Simple USB device detection via /sys/bus/usb/devices
  std::string path = "/sys/bus/usb/devices";
  if (fs::exists(path)) {
    for (const auto &entry : fs::directory_iterator(path)) {
      std::string devPath = entry.path().string();
      std::ifstream prodFile(devPath + "/product");
      std::ifstream manFile(devPath + "/manufacturer");

      if (prodFile.is_open()) {
        std::string product, manufacturer;
        std::getline(prodFile, product);
        std::getline(manFile, manufacturer);

        if (!product.empty()) {
          QVariantMap hw;
          hw["name"] = QString::fromStdString(product);
          hw["vendor"] = QString::fromStdString(manufacturer);
          hw["type"] = "USB Device";
          hw["sysfsPath"] = QString::fromStdString(devPath);

          std::ifstream authFile(devPath + "/authorized");
          int authorized = 1;
          if (authFile.is_open())
            authFile >> authorized;
          hw["enabled"] = (authorized != 0);

          hwList.append(hw);
        }
      }
    }
  }

  // Also check for input devices (peripherals)
  std::string inputPath = "/sys/class/input";
  if (fs::exists(inputPath)) {
    for (const auto &entry : fs::directory_iterator(inputPath)) {
      std::string namePath = entry.path().string() + "/device/name";
      std::ifstream nameFile(namePath);
      if (nameFile.is_open()) {
        std::string name;
        std::getline(nameFile, name);
        if (!name.empty()) {
          QVariantMap hw;
          hw["name"] = QString::fromStdString(name);
          hw["type"] = "Peripheral";
          hw["sysfsPath"] = QString::fromStdString(entry.path().string());
          hw["enabled"] = fs::exists(entry.path().string() + "/device/driver");
          hwList.append(hw);
        }
      }
    }
  }

  if (m_hardware != hwList) {
    m_hardware = hwList;
    emit hardwareChanged();
  }
}

double MetricsService::fetchGpuTemp() {
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

void MetricsService::fetchProcesses() {
  QVariantList procList;
  std::string path = "/proc";
  QList<int> currentPids;

  long long totalTicks = 0;
  {
    std::ifstream statFile("/proc/stat");
    std::string line;
    if (std::getline(statFile, line)) {
      std::istringstream ss(line);
      std::string cpu;
      long long u, n, s, i, io, irq, sirq, st;
      ss >> cpu >> u >> n >> s >> i >> io >> irq >> sirq >> st;
      totalTicks = u + n + s + i + io + irq + sirq + st;
    }
  }

  for (const auto &entry : fs::directory_iterator(path)) {
    if (!entry.is_directory())
      continue;
    std::string filename = entry.path().filename().string();
    if (!std::all_of(filename.begin(), filename.end(), ::isdigit))
      continue;

    int pid = std::stoi(filename);
    currentPids.append(pid);

    // Filter out kernel threads (empty cmdline)
    std::ifstream cmdFile(entry.path().string() + "/cmdline");
    std::string cmdline;
    std::getline(cmdFile, cmdline);
    if (cmdline.empty())
      continue;

    QVariantMap proc;
    proc["pid"] = pid;

    std::ifstream commFile(entry.path().string() + "/comm");
    std::string name;
    std::getline(commFile, name);
    proc["name"] = QString::fromStdString(name);

    long long rss = 0;
    std::ifstream statusFile(entry.path().string() + "/status");
    std::string line;
    while (std::getline(statusFile, line)) {
      if (line.find("VmRSS:") == 0) {
        std::sscanf(line.c_str(), "VmRSS: %lld", &rss);
        proc["memory"] = QString::number(rss / 1024.0, 'f', 1) + " MB";
        proc["memVal"] = (double)rss;
        break;
      }
    }

    double cpuUsage = 0.0;
    std::ifstream pStatFile(entry.path().string() + "/stat");
    if (pStatFile.is_open()) {
      std::string dummy;
      long long utime, stime;
      for (int i = 0; i < 13; ++i)
        pStatFile >> dummy;
      pStatFile >> utime >> stime;

      long long procTicks = utime + stime;
      if (m_prevProcessStates.contains(pid)) {
        long long deltaProc = procTicks - m_prevProcessStates[pid].utime -
                              m_prevProcessStates[pid].stime;
        long long deltaTotal = totalTicks - m_prevProcessStates[pid].total;
        if (deltaTotal > 0) {
          cpuUsage = (double)deltaProc / deltaTotal * 100.0;
        }
      }
      m_prevProcessStates[pid] = {utime, stime, totalTicks};
    }
    proc["cpu"] = QString::number(cpuUsage, 'f', 1) + "%";
    proc["cpuVal"] = cpuUsage;

    procList.append(proc);
  }

  // Cleanup old process states
  for (auto it = m_prevProcessStates.begin();
       it != m_prevProcessStates.end();) {
    if (!currentPids.contains(it.key())) {
      it = m_prevProcessStates.erase(it);
    } else {
      ++it;
    }
  }

  // Sort by CPU usage descending, then by Memory
  std::sort(procList.begin(), procList.end(),
            [](const QVariant &a, const QVariant &b) {
              QVariantMap mapA = a.toMap();
              QVariantMap mapB = b.toMap();
              if (mapA["cpuVal"].toDouble() != mapB["cpuVal"].toDouble())
                return mapA["cpuVal"].toDouble() > mapB["cpuVal"].toDouble();
              return mapA["memVal"].toDouble() > mapB["memVal"].toDouble();
            });

  if (procList.size() > 20) {
    procList = procList.mid(0, 20);
  }

  m_processes = procList;
  emit processesChanged();
}

double MetricsService::fetchUploadSpeed() { return m_uploadSpeed; }
double MetricsService::fetchDownloadSpeed() { return m_downloadSpeed; }

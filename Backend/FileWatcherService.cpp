#include "FileWatcherService.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <algorithm>
#include <errno.h>
#include <filesystem>
#include <fstream>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

namespace fs = std::filesystem;

FileWatcherService::FileWatcherService(QObject *parent) : QObject(parent) {
  m_inotifyFd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
  if (m_inotifyFd < 0) {
    qWarning() << "Failed to initialize inotify:" << strerror(errno);
  }
}

FileWatcherService::~FileWatcherService() {
  if (m_notifier) {
    delete m_notifier;
  }
  if (m_inotifyFd >= 0) {
    ::close(m_inotifyFd);
  }
}

QVariantList FileWatcherService::events() const { return m_events; }

QStringList FileWatcherService::watchedPaths() const { return m_watchedPaths; }

void FileWatcherService::start() {
  if (!m_notifier && m_inotifyFd >= 0) {
    m_notifier = new QSocketNotifier(m_inotifyFd, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated, this,
            &FileWatcherService::readEvents);
    qDebug() << "FileWatcherService SocketNotifier active on FD:"
             << m_inotifyFd;
  }

  // Check if a path is specified in command line arguments (e.g. --watch
  // <path>)
  QString cmdPath;
  QStringList args = QCoreApplication::arguments();
  for (int i = 1; i < args.size() - 1; ++i) {
    if (args[i] == "--watch") {
      cmdPath = args[i + 1];
      break;
    }
  }

  if (!cmdPath.isEmpty()) {
    addPath(cmdPath);
  } else if (m_watchedPaths.isEmpty()) {
    QString defaultPath = "/home/harshit";
    if (!QDir(defaultPath).exists()) {
      defaultPath = QDir::homePath() + "/Downloads";
      if (!QDir(defaultPath).exists()) {
        defaultPath = QDir::homePath();
      }
    }
    addPath(defaultPath);
  }
}

void FileWatcherService::addPath(const QString &path) {
  QString canonicalPath = QDir(path).canonicalPath();
  if (canonicalPath.isEmpty()) {
    qWarning() << "Path does not exist, cannot watch:" << path;
    return;
  }

  if (m_watchedPaths.contains(canonicalPath)) {
    return;
  }

  m_watchedPaths.append(canonicalPath);
  emit watchedPathsChanged();

  addWatchRecursively(canonicalPath);
}

void FileWatcherService::addWatchRecursively(const QString &path) {
  if (shouldIgnore(path))
    return;
  addWatch(path);

  // Traverse and watch subdirectories (capped at 500 to avoid fd exhaustion)
  QDirIterator it(path, QDir::Dirs | QDir::NoDotAndDotDot,
                  QDirIterator::Subdirectories);
  int count = 0;
  while (it.hasNext() && count < 500) {
    QString subDir = it.next();
    if (shouldIgnore(subDir)) {
      continue;
    }
    addWatch(subDir);
    count++;
  }
}

void FileWatcherService::addWatch(const QString &path) {
  if (m_inotifyFd < 0)
    return;

  // We watch for create, modify, delete, access (reads), and moves
  // (rename/trash)
  int wd = inotify_add_watch(m_inotifyFd, path.toUtf8().constData(),
                             IN_CREATE | IN_MODIFY | IN_DELETE | IN_ACCESS |
                                 IN_MOVED_FROM | IN_MOVED_TO);
  if (wd >= 0) {
    m_watchDescriptors[wd] = path;
    qDebug() << "Watching directory:" << path << "(wd:" << wd << ")";
  } else {
    qWarning() << "Failed to watch directory:" << path << strerror(errno);
  }
}

void FileWatcherService::removePath(const QString &path) {
  QString canonicalPath = QDir(path).canonicalPath();
  if (canonicalPath.isEmpty()) {
    canonicalPath = path; // fallback
  }

  if (!m_watchedPaths.contains(canonicalPath)) {
    return;
  }

  m_watchedPaths.removeAll(canonicalPath);
  emit watchedPathsChanged();

  // Find all watch descriptors pointing to this directory or its subdirectories
  QList<int> wdsToRemove;
  for (auto it = m_watchDescriptors.begin(); it != m_watchDescriptors.end();
       ++it) {
    if (it.value() == canonicalPath ||
        it.value().startsWith(canonicalPath + "/")) {
      wdsToRemove.append(it.key());
    }
  }

  for (int wd : wdsToRemove) {
    inotify_rm_watch(m_inotifyFd, wd);
    m_watchDescriptors.remove(wd);
    qDebug() << "Stopped watching wd:" << wd;
  }
}

void FileWatcherService::clearEvents() {
  m_events.clear();
  emit eventsChanged();
}

void FileWatcherService::readEvents() {
  if (m_inotifyFd < 0)
    return;

  alignas(struct inotify_event) char buffer[4096];

  while (true) {
    ssize_t len = ::read(m_inotifyFd, buffer, sizeof(buffer));
    if (len <= 0) {
      if (len < 0 && errno != EAGAIN) {
        qWarning() << "Error reading inotify events:" << strerror(errno);
      }
      break;
    }

    for (char *ptr = buffer; ptr < buffer + len;) {
      struct inotify_event *event =
          reinterpret_cast<struct inotify_event *>(ptr);

      if (event->mask & IN_IGNORED) {
        m_watchDescriptors.remove(event->wd);
      } else if (event->len > 0) {
        QString name = QString::fromUtf8(event->name);
        QString dirPath = m_watchDescriptors.value(event->wd);

        if (!dirPath.isEmpty()) {
          QString fullPath = dirPath + "/" + name;
          QString eventStr;

          if (event->mask & IN_CREATE)
            eventStr = "CREATE";
          else if (event->mask & IN_MODIFY)
            eventStr = "MODIFY";
          else if (event->mask & IN_DELETE)
            eventStr = "DELETE";
          else if (event->mask & IN_MOVED_FROM)
            eventStr = "DELETE";
          else if (event->mask & IN_MOVED_TO)
            eventStr = "CREATE";
          else if (event->mask & IN_ACCESS)
            eventStr = "READ";
          else
            eventStr = "OTHER";

          if (eventStr != "OTHER" && !shouldIgnore(fullPath)) {
            // Resolve PID and process name using procfs
            QPair<int, QString> procInfo = resolveProcess(fullPath);
            QString pidStr =
                (procInfo.first == -1) ? "-" : QString::number(procInfo.first);
            QString procName = procInfo.second;

            addEvent(eventStr, fullPath, pidStr, procName);

            // Dynamically watch newly created/moved subdirectories
            if ((event->mask & IN_ISDIR) &&
                ((event->mask & IN_CREATE) || (event->mask & IN_MOVED_TO))) {
              addWatchRecursively(fullPath);
            }
          }
        }
      }
      ptr += sizeof(struct inotify_event) + event->len;
    }
  }
}

void FileWatcherService::addEvent(const QString &event, const QString &path,
                                  const QString &pid, const QString &process) {
  QVariantMap evt;
  QString timeStr = QDateTime::currentDateTime().toString("hh:mm:ss");
  evt["time"] = timeStr;
  evt["pid"] = pid;
  evt["process"] = process;
  evt["event"] = event;
  evt["path"] = path;

  m_events.prepend(evt);
  while (m_events.size() > 500) {
    m_events.removeLast();
  }
  emit eventsChanged();

  // Print to qDebug() as requested
  qDebug() << QString("[%1] FileEvent - %2 | PID: %3 | Proc: %4 | Path: %5")
                  .arg(timeStr)
                  .arg(event)
                  .arg(pid)
                  .arg(process)
                  .arg(path);
}

QPair<int, QString>
FileWatcherService::resolveProcess(const QString &filePath) {
  std::error_code ec;
  std::string targetPath =
      fs::weakly_canonical(filePath.toStdString(), ec).string();
  if (ec) {
    targetPath = filePath.toStdString();
  }

  // Iterate processes in /proc
  for (const auto &entry : fs::directory_iterator("/proc")) {
    if (!entry.is_directory())
      continue;
    std::string pidStr = entry.path().filename().string();
    if (!std::all_of(pidStr.begin(), pidStr.end(), ::isdigit))
      continue;

    int pid = std::stoi(pidStr);
    std::string fdPath = "/proc/" + pidStr + "/fd";

    try {
      if (fs::exists(fdPath)) {
        for (const auto &fdEntry : fs::directory_iterator(fdPath)) {
          std::error_code sym_ec;
          if (fdEntry.is_symlink()) {
            std::string resolved =
                fs::read_symlink(fdEntry.path(), sym_ec).string();
            if (!sym_ec && resolved == targetPath) {
              // Found! Retrieve process name from /proc/<pid>/comm
              std::ifstream commFile("/proc/" + pidStr + "/comm");
              std::string comm;
              if (std::getline(commFile, comm) && !comm.empty()) {
                return qMakePair(pid, QString::fromStdString(comm));
              }
              return qMakePair(pid, QString("unknown"));
            }
          }
        }
      }
    } catch (...) {
      // Catch filesystem errors / permission denied on system processes
    }
  }

  return qMakePair(-1, QString("unknown"));
}

bool FileWatcherService::shouldIgnore(const QString &path) const {
  // Check if any segment of the path matches ignored folders
  QStringList segments = path.split("/");
  for (const QString &segment : segments) {
    if (segment == "node_modules" || segment == ".git" || segment == "build" ||
        segment == ".gradle" || segment == ".cache" || segment == ".idea" ||
        segment == ".rcc" || segment == "CMakeFiles") {
      return true;
    }
  }
  return false;
}

#include "SyscallTracerService.h"
#include "SyscallTable.h"

#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/reg.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QFileInfo>

// SyscallWorker Implementation

SyscallWorker::SyscallWorker(QObject *parent) : QThread(parent) {
  // Empty
}

SyscallWorker::~SyscallWorker() {
  stop();
}

void SyscallWorker::setupTrace(const QString &binaryPath, const QStringList &args) {
  m_binaryPath = binaryPath;
  m_args = args;
  m_isAttach = false;
}

void SyscallWorker::setupAttach(int pid) {
  m_targetPid = pid;
  m_isAttach = true;
}

void SyscallWorker::stop() {
  m_mutex.lock();
  m_running = false;
  m_mutex.unlock();
}

void SyscallWorker::run() {
  m_mutex.lock();
  m_running = true;
  int pid = m_targetPid;
  bool isAttach = m_isAttach;
  m_mutex.unlock();

  if (isAttach) {
    if (ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) < 0) {
      emit errorOccurred(QString("Failed to attach to PID %1: %2").arg(pid).arg(strerror(errno)));
      emit finishedTracing();
      return;
    }

    int status;
    if (waitpid(pid, &status, 0) < 0) {
      emit errorOccurred(QString("waitpid failed: %1").arg(strerror(errno)));
      ptrace(PTRACE_DETACH, pid, nullptr, nullptr);
      emit finishedTracing();
      return;
    }

    if (ptrace(PTRACE_SETOPTIONS, pid, nullptr, PTRACE_O_TRACESYSGOOD) < 0) {
      emit errorOccurred(QString("Failed to set ptrace options: %1").arg(strerror(errno)));
      ptrace(PTRACE_DETACH, pid, nullptr, nullptr);
      emit finishedTracing();
      return;
    }
  } else {
    // Launch binary
    pid_t child = fork();
    if (child < 0) {
      emit errorOccurred(QString("fork failed: %1").arg(strerror(errno)));
      emit finishedTracing();
      return;
    }

    if (child == 0) {
      // In child process: activate tracing and pause to let parent attach options
      ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
      raise(SIGSTOP);

      // Prepare arguments
      char **argv = new char*[m_args.size() + 2];
      argv[0] = strdup(m_binaryPath.toUtf8().constData());
      for (int i = 0; i < m_args.size(); ++i) {
        argv[i + 1] = strdup(m_args[i].toUtf8().constData());
      }
      argv[m_args.size() + 1] = nullptr;

      execvp(argv[0], argv);

      // If execvp fails
      fprintf(stderr, "Failed to execvp binary: %s\n", strerror(errno));
      _exit(1);
    }

    // In parent process
    pid = child;
    m_mutex.lock();
    m_targetPid = pid;
    m_mutex.unlock();

    int status;
    if (waitpid(pid, &status, 0) < 0) {
      emit errorOccurred(QString("waitpid failed on startup: %1").arg(strerror(errno)));
      emit finishedTracing();
      return;
    }

    if (ptrace(PTRACE_SETOPTIONS, pid, nullptr, PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEEXEC) < 0) {
      emit errorOccurred(QString("Failed to set ptrace options: %1").arg(strerror(errno)));
      emit finishedTracing();
      return;
    }
  }

  int status;
  bool is_syscall_entry = true;

  while (true) {
    m_mutex.lock();
    bool should_run = m_running;
    m_mutex.unlock();
    if (!should_run) break;

    unsigned long signal_to_pass = 0;
    if (WIFSTOPPED(status)) {
      int stopsig = WSTOPSIG(status);
      if (stopsig != (SIGTRAP | 0x80) && stopsig != SIGTRAP) {
        signal_to_pass = stopsig;
      }
    }

    // Continue to next syscall stop
    if (ptrace(PTRACE_SYSCALL, pid, nullptr, signal_to_pass) < 0) {
      if (errno == ESRCH) break; // Process terminated
      break;
    }

    // Wait for state change
    pid_t wpid = waitpid(pid, &status, 0);
    if (wpid < 0) {
      if (errno == EINTR) continue;
      break;
    }

    if (WIFEXITED(status)) {
      emit syscallEvent(QDateTime::currentDateTime().toString("hh:mm:ss"), "exit_group", QString("status=%1").arg(WEXITSTATUS(status)), "0", "process");
      break;
    }
    if (WIFSIGNALED(status)) {
      emit syscallEvent(QDateTime::currentDateTime().toString("hh:mm:ss"), "exit", QString("signal=%1").arg(WTERMSIG(status)), "0", "process");
      break;
    }

    if (WIFSTOPPED(status)) {
      int stopsig = WSTOPSIG(status);

      // System call stop
      if (stopsig == (SIGTRAP | 0x80)) {
        struct user_regs_struct regs;
        if (ptrace(PTRACE_GETREGS, pid, nullptr, &regs) == 0) {
          int syscallNum = regs.orig_rax;
          SyscallInfo info = getSyscallInfo(syscallNum);

          if (is_syscall_entry) {
            is_syscall_entry = false;
          } else {
            is_syscall_entry = true;

            long long retVal = regs.rax;
            QString retStr;
            if (retVal < 0 && retVal >= -4095) {
              retStr = QString("-1 (%1)").arg(strerror(-retVal));
            } else {
              if (retVal > 100000) {
                retStr = QString("0x%1").arg(retVal, 0, 16);
              } else {
                retStr = QString::number(retVal);
              }
            }

            QString argStr = formatSyscallArgs(pid, syscallNum, regs);
            QString timeStr = QDateTime::currentDateTime().toString("hh:mm:ss");

            emit syscallEvent(timeStr, info.name, argStr, retStr, info.category);
          }
        }
      }
      else if (stopsig == SIGTRAP) {
        // Just let it continue (e.g. execve trap)
      }
    }
  }

  // Detach / Cleanup
  if (isAttach) {
    ptrace(PTRACE_DETACH, pid, nullptr, nullptr);
  } else {
    // If we launched the child, terminate if still alive
    kill(pid, SIGKILL);
  }

  emit finishedTracing();
}

QString SyscallWorker::readStringFromTracee(int pid, unsigned long long addr) {
  if (addr == 0) return "NULL";

  // Read from /proc/<pid>/mem
  QString memPath = QString("/proc/%1/mem").arg(pid);
  int fd = ::open(memPath.toUtf8().constData(), O_RDONLY);
  if (fd >= 0) {
    if (::lseek64(fd, addr, SEEK_SET) == (off_t)addr) {
      std::string result;
      char buf[256];
      bool done = false;
      while (!done) {
        ssize_t bytes = ::read(fd, buf, sizeof(buf));
        if (bytes <= 0) break;
        for (ssize_t i = 0; i < bytes; ++i) {
          if (buf[i] == '\0') {
            done = true;
            break;
          }
          result.push_back(buf[i]);
        }
        if (result.size() > 2048) { // Safeguard
          result += "...";
          break;
        }
      }
      ::close(fd);
      return QString::fromStdString(result);
    }
    ::close(fd);
  }

  // Fallback to PTRACE_PEEKDATA
  QString result;
  unsigned long long offset = 0;
  bool done = false;
  while (!done && offset < 1024) {
    long val = ptrace(PTRACE_PEEKDATA, pid, addr + offset, nullptr);
    if (val == -1 && errno != 0) break;

    char *chars = reinterpret_cast<char*>(&val);
    for (size_t i = 0; i < sizeof(long); ++i) {
      if (chars[i] == '\0') {
        done = true;
        break;
      }
      result.append(chars[i]);
    }
    offset += sizeof(long);
  }
  return result;
}

QString SyscallWorker::readSockAddr(int pid, unsigned long long addr, unsigned long long len) {
  if (addr == 0 || len == 0) return "NULL";

  QByteArray data;
  data.resize(len);

  // Read from /proc/<pid>/mem
  QString memPath = QString("/proc/%1/mem").arg(pid);
  int fd = ::open(memPath.toUtf8().constData(), O_RDONLY);
  bool read_success = false;
  if (fd >= 0) {
    if (::lseek64(fd, addr, SEEK_SET) == (off_t)addr) {
      ssize_t bytes = ::read(fd, data.data(), len);
      if (bytes == (ssize_t)len) {
        read_success = true;
      }
    }
    ::close(fd);
  }

  // Fallback to PTRACE_PEEKDATA
  if (!read_success) {
    unsigned long long offset = 0;
    while (offset < len) {
      long val = ptrace(PTRACE_PEEKDATA, pid, addr + offset, nullptr);
      if (val == -1 && errno != 0) break;
      size_t chunk = std::min<size_t>(sizeof(long), len - offset);
      memcpy(data.data() + offset, &val, chunk);
      offset += sizeof(long);
    }
  }

  const struct sockaddr *sa = reinterpret_cast<const struct sockaddr *>(data.constData());
  if (sa->sa_family == AF_INET) {
    const struct sockaddr_in *sin = reinterpret_cast<const struct sockaddr_in *>(sa);
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(sin->sin_addr), ip, INET_ADDRSTRLEN);
    unsigned short port = ntohs(sin->sin_port);
    return QString("%1:%2").arg(ip).arg(port);
  } else if (sa->sa_family == AF_INET6) {
    const struct sockaddr_in6 *sin6 = reinterpret_cast<const struct sockaddr_in6 *>(sa);
    char ip[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &(sin6->sin6_addr), ip, INET6_ADDRSTRLEN);
    unsigned short port = ntohs(sin6->sin6_port);
    return QString("[%1]:%2").arg(ip).arg(port);
  } else if (sa->sa_family == AF_UNIX) {
    const struct sockaddr_un *sun = reinterpret_cast<const struct sockaddr_un *>(sa);
    return QString("unix:%1").arg(sun->sun_path);
  }

  return QString("family=%1").arg(sa->sa_family);
}

QString SyscallWorker::formatSyscallArgs(int pid, int syscallNum, const struct user_regs_struct &regs) {
  unsigned long long arg1 = regs.rdi;
  unsigned long long arg2 = regs.rsi;
  unsigned long long arg3 = regs.rdx;
  unsigned long long arg4 = regs.r10;
  unsigned long long arg5 = regs.r8;

  switch (syscallNum) {
    case 0: // read
      return QString("fd=%1, count=%2").arg((int)arg1).arg(arg3);

    case 1: // write
      return QString("fd=%1, count=%2").arg((int)arg1).arg(arg3);

    case 2: // open
    case 85: // creat
    {
      QString path = readStringFromTracee(pid, arg1);
      int flags = (int)arg2;
      QString flagsStr;
      if (flags == 0) flagsStr = "O_RDONLY";
      else {
        QStringList fl;
        if (flags & O_WRONLY) fl << "O_WRONLY";
        if (flags & O_RDWR) fl << "O_RDWR";
        if (flags & O_CREAT) fl << "O_CREAT";
        if (flags & O_EXCL) fl << "O_EXCL";
        if (flags & O_TRUNC) fl << "O_TRUNC";
        if (flags & O_APPEND) fl << "O_APPEND";
        flagsStr = fl.join("|");
        if (flagsStr.isEmpty()) flagsStr = QString("0x%1").arg(flags, 0, 16);
      }
      return QString("\"%1\", %2").arg(path).arg(flagsStr);
    }

    case 257: // openat
    {
      int dirfd = (int)arg1;
      QString dirfdStr = (dirfd == -100) ? "AT_FDCWD" : QString::number(dirfd);
      QString path = readStringFromTracee(pid, arg2);
      int flags = (int)arg3;
      QString flagsStr;
      if (flags == 0) flagsStr = "O_RDONLY";
      else {
        QStringList fl;
        if (flags & O_WRONLY) fl << "O_WRONLY";
        if (flags & O_RDWR) fl << "O_RDWR";
        if (flags & O_CREAT) fl << "O_CREAT";
        if (flags & O_EXCL) fl << "O_EXCL";
        if (flags & O_TRUNC) fl << "O_TRUNC";
        if (flags & O_APPEND) fl << "O_APPEND";
        flagsStr = fl.join("|");
        if (flagsStr.isEmpty()) flagsStr = QString("0x%1").arg(flags, 0, 16);
      }
      return QString("%1, \"%2\", %3").arg(dirfdStr).arg(path).arg(flagsStr);
    }

    case 59: // execve
    {
      QString path = readStringFromTracee(pid, arg1);
      QStringList args;
      unsigned long long argvAddr = arg2;
      if (argvAddr != 0) {
        for (int i = 0; i < 100; ++i) {
          unsigned long long strAddr = 0;
          QString memPath = QString("/proc/%1/mem").arg(pid);
          int fd = ::open(memPath.toUtf8().constData(), O_RDONLY);
          bool read_success = false;
          if (fd >= 0) {
            if (::lseek64(fd, argvAddr + i * sizeof(char*), SEEK_SET) == (off_t)(argvAddr + i * sizeof(char*))) {
              if (::read(fd, &strAddr, sizeof(char*)) == sizeof(char*)) {
                read_success = true;
              }
            }
            ::close(fd);
          }
          if (!read_success) {
            strAddr = ptrace(PTRACE_PEEKDATA, pid, argvAddr + i * sizeof(char*), nullptr);
          }
          if (strAddr == 0) break;
          args << "\"" + readStringFromTracee(pid, strAddr) + "\"";
        }
      }
      return QString("\"%1\", [%2]").arg(path).arg(args.join(", "));
    }

    case 42: // connect
    case 49: // bind
    {
      int sockfd = (int)arg1;
      QString addrStr = readSockAddr(pid, arg2, arg3);
      return QString("fd=%1, %2").arg(sockfd).arg(addrStr);
    }

    case 9: // mmap
    {
      unsigned long long addr = arg1;
      unsigned long long len = arg2;
      int prot = (int)arg3;
      int flags = (int)arg4;
      int fd = (int)arg5;

      QString protStr;
      QStringList protList;
      if (prot == 0) protStr = "PROT_NONE";
      else {
        if (prot & 1) protList << "PROT_READ";
        if (prot & 2) protList << "PROT_WRITE";
        if (prot & 4) protList << "PROT_EXEC";
        protStr = protList.join("|");
      }

      QString flagsStr;
      QStringList flagsList;
      if (flags & 0x01) flagsList << "MAP_SHARED";
      if (flags & 0x02) flagsList << "MAP_PRIVATE";
      if (flags & 0x10) flagsList << "MAP_ANONYMOUS";
      flagsStr = flagsList.join("|");
      if (flagsStr.isEmpty()) flagsStr = QString("0x%1").arg(flags, 0, 16);

      return QString("addr=0x%1, len=%2, prot=%3, flags=%4, fd=%5")
          .arg(addr, 0, 16).arg(len).arg(protStr).arg(flagsStr).arg(fd);
    }

    default:
      return QString("0x%1, 0x%2, 0x%3").arg(arg1, 0, 16).arg(arg2, 0, 16).arg(arg3, 0, 16);
  }
}

// SyscallTracerService Implementation

SyscallTracerService::SyscallTracerService(QObject *parent) : QObject(parent) {
  // Empty
}

SyscallTracerService::~SyscallTracerService() {
  detach();
}

QVariantList SyscallTracerService::events() const {
  return m_events;
}

bool SyscallTracerService::active() const {
  return m_active;
}

int SyscallTracerService::targetPid() const {
  return m_targetPid;
}

QString SyscallTracerService::targetName() const {
  return m_targetName;
}

void SyscallTracerService::traceBinary(const QString &binaryPath, const QStringList &args) {
  if (m_active) return;

  m_active = true;
  m_targetPid = -1;
  m_targetName = QFileInfo(binaryPath).fileName();
  m_uniqueSyscalls.clear();
  m_events.clear();

  emit activeChanged();
  emit targetPidChanged();
  emit targetNameChanged();
  emit eventsChanged();

  m_worker = new SyscallWorker(this);
  m_worker->setupTrace(binaryPath, args);

  connect(m_worker, &SyscallWorker::syscallEvent, this, &SyscallTracerService::handleSyscallEvent);
  connect(m_worker, &SyscallWorker::errorOccurred, this, &SyscallTracerService::handleError);
  connect(m_worker, &SyscallWorker::finishedTracing, this, &SyscallTracerService::handleFinishedTracing);

  m_worker->start();
}

void SyscallTracerService::attachPid(int pid, const QString &processName) {
  if (m_active) return;

  m_active = true;
  m_targetPid = pid;
  m_targetName = processName;
  m_uniqueSyscalls.clear();
  m_events.clear();

  emit activeChanged();
  emit targetPidChanged();
  emit targetNameChanged();
  emit eventsChanged();

  m_worker = new SyscallWorker(this);
  m_worker->setupAttach(pid);

  connect(m_worker, &SyscallWorker::syscallEvent, this, &SyscallTracerService::handleSyscallEvent);
  connect(m_worker, &SyscallWorker::errorOccurred, this, &SyscallTracerService::handleError);
  connect(m_worker, &SyscallWorker::finishedTracing, this, &SyscallTracerService::handleFinishedTracing);

  m_worker->start();
}

void SyscallTracerService::detach() {
  if (!m_active) return;

  if (m_worker) {
    m_worker->stop();
    m_worker->wait();
    delete m_worker;
    m_worker = nullptr;
  }

  m_active = false;
  emit activeChanged();
}

void SyscallTracerService::clearEvents() {
  m_events.clear();
  emit eventsChanged();
}

void SyscallTracerService::handleSyscallEvent(const QString &time, const QString &name, const QString &args, const QString &ret, const QString &category) {
  QVariantMap evt;
  evt["time"] = time;
  evt["syscall"] = name;
  evt["args"] = args;
  evt["ret"] = ret;
  evt["category"] = category;

  m_events.prepend(evt);
  while (m_events.size() > 2000) {
    m_events.removeLast();
  }
  emit eventsChanged();

  m_uniqueSyscalls.insert(name);
  qDebug() << QString("[%1] Syscall: %2(%3) -> %4").arg(time).arg(name).arg(args).arg(ret);
}

void SyscallTracerService::handleFinishedTracing() {
  m_active = false;
  emit activeChanged();
}

void SyscallTracerService::handleError(const QString &errorMsg) {
  qWarning() << "SyscallTracer Error:" << errorMsg;
  handleSyscallEvent(QDateTime::currentDateTime().toString("hh:mm:ss"), "ERROR", errorMsg, "-1", "other");
}

void SyscallTracerService::exportSeccompPolicy(const QString &saveUrl) {
  QString localPath = saveUrl;
  if (localPath.startsWith("file://")) {
    localPath = QUrl(saveUrl).toLocalFile();
  }

  if (localPath.isEmpty()) return;

  QFile file(localPath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qWarning() << "Failed to open seccomp export file:" << file.errorString();
    return;
  }

  QJsonObject root;
  root["defaultAction"] = "SCMP_ACT_ERRNO";

  QJsonArray architectures;
  architectures.append("SCMP_ARCH_X86_64");
  root["architectures"] = architectures;

  QJsonArray syscalls;
  QJsonObject rule;

  QJsonArray names;
  for (const QString &sc : m_uniqueSyscalls) {
    if (sc != "ERROR" && sc != "exit" && sc != "exit_group" && sc != "unknown" && sc != "exit_group") {
      names.append(sc);
    }
  }

  // Ensure minimum viable process syscalls
  if (!m_uniqueSyscalls.contains("exit_group")) {
    names.append("exit_group");
  }

  rule["names"] = names;
  rule["action"] = "SCMP_ACT_ALLOW";
  rule["args"] = QJsonArray();

  syscalls.append(rule);
  root["syscalls"] = syscalls;

  QJsonDocument doc(root);
  file.write(doc.toJson(QJsonDocument::Indented));
  file.close();
  qDebug() << "Successfully exported seccomp policy to" << localPath;
}

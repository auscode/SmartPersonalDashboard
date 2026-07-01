#pragma once

#include <QObject>
#include <QThread>
#include <QStringList>
#include <QVariantList>
#include <QSet>
#include <QMutex>

// Forward declarations
struct user_regs_struct;

class SyscallWorker : public QThread {
  Q_OBJECT
public:
  explicit SyscallWorker(QObject *parent = nullptr);
  ~SyscallWorker();

  void setupTrace(const QString &binaryPath, const QStringList &args);
  void setupAttach(int pid);
  void stop();

signals:
  void syscallEvent(const QString &time, const QString &name, const QString &args, const QString &ret, const QString &category);
  void errorOccurred(const QString &errorMsg);
  void finishedTracing();

protected:
  void run() override;

private:
  QString m_binaryPath;
  QStringList m_args;
  int m_targetPid = -1;
  bool m_isAttach = false;
  volatile bool m_running = false;
  QMutex m_mutex;

  QString readStringFromTracee(int pid, unsigned long long addr);
  QString readSockAddr(int pid, unsigned long long addr, unsigned long long len);
  QString formatSyscallArgs(int pid, int syscallNum, const struct user_regs_struct &regs);
};

class SyscallTracerService : public QObject {
  Q_OBJECT
  Q_PROPERTY(QVariantList events READ events NOTIFY eventsChanged)
  Q_PROPERTY(bool active READ active NOTIFY activeChanged)
  Q_PROPERTY(int targetPid READ targetPid NOTIFY targetPidChanged)
  Q_PROPERTY(QString targetName READ targetName NOTIFY targetNameChanged)

public:
  explicit SyscallTracerService(QObject *parent = nullptr);
  ~SyscallTracerService();

  QVariantList events() const;
  bool active() const;
  int targetPid() const;
  QString targetName() const;

  Q_INVOKABLE void traceBinary(const QString &binaryPath, const QStringList &args);
  Q_INVOKABLE void attachPid(int pid, const QString &processName);
  Q_INVOKABLE void detach();
  Q_INVOKABLE void clearEvents();
  Q_INVOKABLE void exportSeccompPolicy(const QString &saveUrl);

signals:
  void eventsChanged();
  void activeChanged();
  void targetPidChanged();
  void targetNameChanged();

private slots:
  void handleSyscallEvent(const QString &time, const QString &name, const QString &args, const QString &ret, const QString &category);
  void handleFinishedTracing();
  void handleError(const QString &errorMsg);

private:
  QVariantList m_events;
  bool m_active = false;
  int m_targetPid = -1;
  QString m_targetName;
  QSet<QString> m_uniqueSyscalls;
  SyscallWorker *m_worker = nullptr;
};

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QSocketNotifier>
#include <QMap>
#include <QPair>

class FileWatcherService : public QObject {
  Q_OBJECT
  Q_PROPERTY(QVariantList events READ events NOTIFY eventsChanged)
  Q_PROPERTY(QStringList watchedPaths READ watchedPaths NOTIFY watchedPathsChanged)
// wire up the QML table, add multi-path support, cap events at 500, polish the UI to match your existing dark theme.
public:
  explicit FileWatcherService(QObject *parent = nullptr);
  ~FileWatcherService();

  QVariantList events() const;
  QStringList watchedPaths() const;

  Q_INVOKABLE void start();
  Q_INVOKABLE void addPath(const QString &path);
  Q_INVOKABLE void removePath(const QString &path);
  Q_INVOKABLE void clearEvents();

signals:
  void eventsChanged();
  void watchedPathsChanged();

private slots:
  void readEvents();

private:
  int m_inotifyFd = -1;
  QSocketNotifier *m_notifier = nullptr;
  QMap<int, QString> m_watchDescriptors; // Maps wd -> directory path
  QVariantList m_events;
  QStringList m_watchedPaths;

  void addWatch(const QString &path);
  void addWatchRecursively(const QString &path);
  void addEvent(const QString &event, const QString &path, const QString &pid, const QString &process);
  QPair<int, QString> resolveProcess(const QString &filePath);
  bool shouldIgnore(const QString &path) const;
};

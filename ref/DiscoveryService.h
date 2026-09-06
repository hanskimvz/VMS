#pragma once

#include "core/DeviceRecord.h"

#include <QObject>
#include <QThread>

class DiscoveryWorker;

class DiscoveryService : public QObject
{
    Q_OBJECT

public:
    explicit DiscoveryService(QObject *parent = nullptr);
    ~DiscoveryService() override;

    bool isRunning() const { return m_running; }

public slots:
    void startDiscovery(const QString &ipRange = QString());

signals:
    void discoveryStarted();
    void statusChanged(const QString &message);
    void logLine(const QString &line);
    void discoveryFinished(const QList<DeviceRecord> &devices);
    void discoveryFailed(const QString &error);

private:
    void onWorkerFinished(const QList<DeviceRecord> &devices);
    void onWorkerFailed(const QString &error);

    QThread m_thread;
    DiscoveryWorker *m_worker = nullptr;
    bool m_running = false;
};

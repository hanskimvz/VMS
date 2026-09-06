#include "io/DiscoveryService.h"

#include "io/DiscoveryWorker.h"

DiscoveryService::DiscoveryService(QObject *parent)
    : QObject(parent)
{
    m_worker = new DiscoveryWorker();
    m_worker->moveToThread(&m_thread);

    connect(&m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_worker, &DiscoveryWorker::statusChanged, this, &DiscoveryService::statusChanged);
    connect(m_worker, &DiscoveryWorker::logLine, this, &DiscoveryService::logLine);
    connect(m_worker, &DiscoveryWorker::finished, this, &DiscoveryService::onWorkerFinished);
    connect(m_worker, &DiscoveryWorker::failed, this, &DiscoveryService::onWorkerFailed);

    m_thread.start();
}

DiscoveryService::~DiscoveryService()
{
    m_thread.quit();
    m_thread.wait();
}

void DiscoveryService::startDiscovery(const QString &ipRange)
{
    if (m_running) {
        return;
    }

    m_running = true;
    emit discoveryStarted();
    QMetaObject::invokeMethod(m_worker, "runDiscovery", Qt::QueuedConnection,
                              Q_ARG(QString, ipRange.trimmed()));
}

void DiscoveryService::onWorkerFinished(const QList<DeviceRecord> &devices)
{
    m_running = false;
    emit discoveryFinished(devices);
}

void DiscoveryService::onWorkerFailed(const QString &error)
{
    m_running = false;
    emit discoveryFailed(error);
}

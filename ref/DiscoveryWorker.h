#pragma once

#include "core/DeviceRecord.h"

#include <QObject>

class DiscoveryWorker : public QObject
{
    Q_OBJECT

public:
    explicit DiscoveryWorker(QObject *parent = nullptr);

public slots:
    void runDiscovery(const QString &ipRange = QString());

signals:
    void statusChanged(const QString &message);
    void logLine(const QString &line);
    void finished(const QList<DeviceRecord> &devices);
    void failed(const QString &error);
};

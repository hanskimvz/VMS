#ifndef DEVICE_DISCOVERY_H
#define DEVICE_DISCOVERY_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QSet>
#include <QString>

struct DiscoveredDevice {
    QString ip;
    QString name;
    QString model;
    QString manufacturer;
    QString serviceUrl;
    QString discoveryType;  // "ONVIF", "mDNS", "UPnP"
    int port = 0;
};

class DeviceDiscovery : public QObject {
    Q_OBJECT

public:
    explicit DeviceDiscovery(QObject* parent = nullptr);
    ~DeviceDiscovery();

    void startDiscovery(int timeout = 5000);
    void stopDiscovery();
    bool isDiscovering() const { return m_discovering; }

signals:
    void deviceDiscovered(const DiscoveredDevice& device);
    void discoveryFinished();
    void error(const QString& message);

private slots:
    void onMdnsReadyRead();
    void onSsdpReadyRead();
    void onWsDiscoveryReadyRead();
    void onDiscoveryTimeout();

private:
    void startMdnsDiscovery();
    void startSsdpDiscovery();
    void startWsDiscovery();
    void parseMdnsResponse(const QByteArray& data, const QHostAddress& sender);
    void parseSsdpResponse(const QByteArray& data, const QHostAddress& sender);
    void parseWsDiscoveryResponse(const QByteArray& data, const QHostAddress& sender);
    
    QUdpSocket* m_mdnsSocket = nullptr;
    QUdpSocket* m_ssdpSocket = nullptr;
    QUdpSocket* m_wsDiscoverySocket = nullptr;
    QTimer* m_discoveryTimer = nullptr;
    bool m_discovering = false;
    
    QSet<QString> m_discoveredIPs;
    
    static constexpr const char* MDNS_ADDRESS = "224.0.0.251";
    static constexpr quint16 MDNS_PORT = 5353;
    static constexpr const char* SSDP_ADDRESS = "239.255.255.250";
    static constexpr quint16 SSDP_PORT = 1900;
    static constexpr const char* WS_DISCOVERY_ADDRESS = "239.255.255.250";
    static constexpr quint16 WS_DISCOVERY_PORT = 3702;
};

#endif // DEVICE_DISCOVERY_H

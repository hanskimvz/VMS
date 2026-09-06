#ifndef DEVICE_DISCOVERY_H
#define DEVICE_DISCOVERY_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QHostAddress>
#include <QList>
#include <QSet>
#include <QString>

struct DiscoveredDevice {
    QString ip;
    QString name;
    QString model;
    QString manufacturer;
    QString serviceUrl;
    QString discoveryType;  // "mDNS", "UPnP"
    int port = 0;
};

// mDNS 와 SSDP(UPnP) 로 카메라 후보를 찾는다.
// ONVIF WS-Discovery 는 OnvifClient::discover() 가 담당한다.
// 프로브는 사용 가능한 모든 IPv4 인터페이스(또는 지정한 인터페이스)로 각각 보낸다.
class DeviceDiscovery : public QObject {
    Q_OBJECT

public:
    explicit DeviceDiscovery(QObject* parent = nullptr);
    ~DeviceDiscovery();

    // localAddresses 가 비어 있으면 모든 인터페이스를 쓴다.
    void startDiscovery(int timeout = 5000, const QList<QHostAddress>& localAddresses = QList<QHostAddress>());
    void stopDiscovery();
    bool isDiscovering() const { return m_discovering; }

signals:
    void deviceDiscovered(const DiscoveredDevice& device);
    void discoveryFinished();
    void info(const QString& message);
    void error(const QString& message);

private slots:
    void onMdnsReadyRead();
    void onSsdpReadyRead();
    void onDiscoveryTimeout();
    void sendQueries();

private:
    void parseMdnsResponse(const QByteArray& data, const QHostAddress& sender);
    void parseSsdpResponse(const QByteArray& data, const QHostAddress& sender);
    static QByteArray buildMdnsQuery();

    QList<QUdpSocket*> m_ssdpSockets;     // 인터페이스당 1개. 응답은 유니캐스트로 돌아온다
    QList<QUdpSocket*> m_mdnsSockets;     // 인터페이스당 1개. QU 비트로 유니캐스트 응답을 요청한다
    QUdpSocket* m_mdnsListener = nullptr; // 5353 멀티캐스트 응답 수신용(best effort)
    QTimer* m_discoveryTimer = nullptr;
    QTimer* m_resendTimer = nullptr;
    bool m_discovering = false;

    QSet<QString> m_discoveredIPs;

    static constexpr const char* MDNS_ADDRESS = "224.0.0.251";
    static constexpr quint16 MDNS_PORT = 5353;
    static constexpr const char* SSDP_ADDRESS = "239.255.255.250";
    static constexpr quint16 SSDP_PORT = 1900;
};

#endif // DEVICE_DISCOVERY_H

#ifndef ONVIF_CLIENT_H
#define ONVIF_CLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUdpSocket>
#include <QHostAddress>
#include <QList>
#include <QSet>
#include <QTimer>
#include <QMap>

struct OnvifDevice {
    QString address;
    QString name;
    QString manufacturer;
    QString model;
    QString xaddr;
    QString serviceUrl;
    QString serialNumber;
    QString firmwareVersion;
};

struct OnvifCapabilities {
    QString mediaServiceUrl;
    QString ptzServiceUrl;
    QString imagingServiceUrl;
    QString eventsServiceUrl;
    QString deviceServiceUrl;
};

struct OnvifDeviceInfo {
    QString manufacturer;
    QString model;
    QString firmwareVersion;
    QString serialNumber;
    QString hardwareId;
};

struct OnvifProfile {
    QString token;
    QString name;
    int width = 0;
    int height = 0;
    int fps = 0;
    QString encoding;
    QString streamUri;
};

enum class PtzAction {
    Stop,
    Up,
    Down,
    Left,
    Right,
    UpLeft,
    UpRight,
    DownLeft,
    DownRight,
    ZoomIn,
    ZoomOut
};

struct NetworkInterface {
    QString token;           // Interface token (e.g., "eth0")
    QString name;            // Friendly name
    QString macAddress;
    bool enabled = true;
    
    // IPv4 settings
    bool dhcpEnabled = false;
    QString ipAddress;
    int prefixLength = 24;   // Subnet mask as prefix (24 = 255.255.255.0)
    QString gateway;
    
    // DNS
    QStringList dnsServers;
    
    // For response
    bool rebootNeeded = false;
};

class OnvifClient : public QObject {
    Q_OBJECT
    
public:
    explicit OnvifClient(QObject* parent = nullptr);
    ~OnvifClient();
    
    // WS-Discovery 프로브를 보낸다. localAddresses 가 비어 있으면 사용 가능한 모든 IPv4 인터페이스로 보낸다.
    void discover(int timeout = 3000, const QList<QHostAddress>& localAddresses = QList<QHostAddress>());
    void stopDiscovery();
    
    void setCredentials(const QString& username, const QString& password);
    
    void getCapabilities(const QString& deviceServiceUrl);
    void getServices(const QString& deviceServiceUrl);
    void getDeviceInformation(const QString& deviceServiceUrl);
    void getProfiles(const QString& mediaServiceUrl);
    void getStreamUri(const QString& mediaServiceUrl, const QString& profileToken);
    void ptzMove(const QString& ptzServiceUrl, const QString& profileToken, 
                 PtzAction action, float speed = 0.5f);
    
    // Network management
    void getNetworkInterfaces(const QString& deviceServiceUrl);
    void setNetworkInterfaces(const QString& deviceServiceUrl, const NetworkInterface& config);
    void getNetworkDefaultGateway(const QString& deviceServiceUrl);
    void setNetworkDefaultGateway(const QString& deviceServiceUrl, const QString& gateway);
    void systemReboot(const QString& deviceServiceUrl);
    
    // 현재 연결 정보
    OnvifCapabilities currentCapabilities() const { return m_capabilities; }
    QList<OnvifProfile> currentProfiles() const { return m_profiles; }
    QString currentStreamUri() const { return m_currentStreamUri; }
    
signals:
    void deviceDiscovered(const OnvifDevice& device);
    void discoveryFinished();
    void discoveryInfo(const QString& message);   // 어느 인터페이스로 보냈는지 등 진행 정보
    void capabilitiesReceived(const OnvifCapabilities& capabilities);
    void deviceInformationReceived(const OnvifDeviceInfo& info);
    void profilesReceived(const QList<OnvifProfile>& profiles);
    void streamUriReceived(const QString& profileToken, const QString& uri);
    void networkInterfacesReceived(const QList<NetworkInterface>& interfaces);
    void networkSettingsChanged(bool rebootNeeded);
    void systemRebooting();
    void error(const QString& message);
    
private slots:
    void onDiscoveryReadyRead();
    void onDiscoveryTimeout();
    void onHttpFinished(QNetworkReply* reply);
    
private:
    QString createWsDiscoveryProbe() const;
    QString createGetCapabilitiesRequest() const;
    QString createGetServicesRequest() const;
    QString createGetDeviceInformationRequest() const;
    QString createGetProfilesRequest() const;
    QString createGetStreamUriRequest(const QString& profileToken) const;
    QString createPtzMoveRequest(const QString& profileToken, PtzAction action, float speed) const;
    QString createPtzStopRequest(const QString& profileToken) const;
    
    QString createSoapEnvelope(const QString& body) const;
    QString createSecurityHeader() const;
    
    void sendDiscoveryProbe();
    void parseDiscoveryResponse(const QByteArray& data, const QHostAddress& sender);
    void parseCapabilitiesResponse(const QByteArray& data);
    void parseServicesResponse(const QByteArray& data);
    void parseDeviceInformationResponse(const QByteArray& data);
    void parseProfilesResponse(const QByteArray& data);
    void parseStreamUriResponse(const QByteArray& data, const QString& profileToken);
    void parseNetworkInterfacesResponse(const QByteArray& data);
    void parseSetNetworkInterfacesResponse(const QByteArray& data);
    
    QString createGetNetworkInterfacesRequest() const;
    QString createSetNetworkInterfacesRequest(const NetworkInterface& config) const;
    QString createSetNetworkDefaultGatewayRequest(const QString& gateway) const;
    QString createSystemRebootRequest() const;
    
    void retryRequest(QNetworkReply* reply);
    
    QList<QUdpSocket*> m_discoverySockets;    // 인터페이스당 1개
    QSet<QString> m_discoveredAddresses;      // 이번 검색에서 이미 보고한 장치 IP
    QNetworkAccessManager* m_networkManager;
    QTimer* m_discoveryTimer;
    QTimer* m_retryTimer = nullptr;
    
    QString m_username;
    QString m_password;
    
    OnvifCapabilities m_capabilities;
    QList<OnvifProfile> m_profiles;
    QString m_currentStreamUri;
    QString m_currentMediaUrl;
    
    // Retry handling
    QMap<QString, int> m_retryCount;  // URL -> retry count
    static const int MAX_RETRIES = 3;
    static const int RETRY_DELAY_MS = 1000;
    
    static const QString WS_DISCOVERY_ADDRESS;
    static const int WS_DISCOVERY_PORT = 3702;
};

#endif // ONVIF_CLIENT_H

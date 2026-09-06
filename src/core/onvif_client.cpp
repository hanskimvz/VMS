#include "onvif_client.h"
#include <QCryptographicHash>
#include <QDateTime>
#include <QRandomGenerator>
#include <QXmlStreamReader>
#include <QDebug>
#include <QUuid>
#include <QUrl>
#include <QAuthenticator>
#include <QMap>
#include <QNetworkDatagram>
#include <QSet>
#include "local_interfaces.h"

const QString OnvifClient::WS_DISCOVERY_ADDRESS = "239.255.255.250";

OnvifClient::OnvifClient(QObject* parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_discoveryTimer(new QTimer(this))
{
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &OnvifClient::onHttpFinished);
    connect(m_discoveryTimer, &QTimer::timeout,
            this, &OnvifClient::onDiscoveryTimeout);
    
    // Handle HTTP Digest Authentication
    connect(m_networkManager, &QNetworkAccessManager::authenticationRequired,
            this, [this](QNetworkReply* reply, QAuthenticator* authenticator) {
                Q_UNUSED(reply);
                qDebug() << "Authentication required, providing credentials";
                authenticator->setUser(m_username);
                authenticator->setPassword(m_password);
            });
    
    m_discoveryTimer->setSingleShot(true);
}

OnvifClient::~OnvifClient() {
    stopDiscovery();
}

void OnvifClient::setCredentials(const QString& username, const QString& password) {
    m_username = username;
    m_password = password;
}

void OnvifClient::discover(int timeout, const QList<QHostAddress>& localAddresses) {
    stopDiscovery();
    m_discoveredAddresses.clear();

    // 소켓을 0.0.0.0 에 바인드해서 멀티캐스트를 보내면 OS 는 기본 경로 인터페이스로만 내보낸다.
    // VPN 이나 가상 어댑터(Hyper-V, WSL)가 기본 경로를 잡고 있으면 카메라가 있는 LAN 에는
    // 프로브가 나가지 않는다. 그래서 인터페이스마다 소켓을 만들고 송신 인터페이스를 명시한다.
    const QList<LocalInterface> interfaces = resolveDiscoveryInterfaces(localAddresses);
    if (interfaces.isEmpty()) {
        emit error("No usable IPv4 network interface for discovery");
        emit discoveryFinished();
        return;
    }

    for (const LocalInterface& li : interfaces) {
        auto* socket = new QUdpSocket(this);
        if (!bindDiscoverySocket(*socket, li)) {
            delete socket;
            continue;
        }
        connect(socket, &QUdpSocket::readyRead, this, &OnvifClient::onDiscoveryReadyRead);
        m_discoverySockets.append(socket);
        emit discoveryInfo(QString("WS-Discovery on %1").arg(li.label()));
    }

    if (m_discoverySockets.isEmpty()) {
        emit error("Failed to bind discovery socket on any interface");
        emit discoveryFinished();
        return;
    }

    qInfo() << "OnvifClient: discovery on" << m_discoverySockets.size()
            << "interface(s), timeout" << timeout << "ms";

    sendDiscoveryProbe();
    // 일부 카메라는 첫 프로브를 놓친다. 1초 뒤 한 번 더 보낸다.
    QTimer::singleShot(1000, this, [this]() {
        if (!m_discoverySockets.isEmpty()) {
            sendDiscoveryProbe();
        }
    });

    m_discoveryTimer->start(timeout);
}

void OnvifClient::sendDiscoveryProbe() {
    const QByteArray data = createWsDiscoveryProbe().toUtf8();
    for (QUdpSocket* socket : m_discoverySockets) {
        qint64 sent = socket->writeDatagram(data, QHostAddress(WS_DISCOVERY_ADDRESS), WS_DISCOVERY_PORT);
        if (sent < 0) {
            qWarning() << "OnvifClient: probe send failed from" << socket->localAddress().toString()
                       << socket->errorString();
        } else {
            qDebug() << "OnvifClient: probe sent from" << socket->localAddress().toString();
        }
    }
}

void OnvifClient::stopDiscovery() {
    m_discoveryTimer->stop();

    for (QUdpSocket* socket : m_discoverySockets) {
        socket->close();
        socket->deleteLater();
    }
    m_discoverySockets.clear();
}

void OnvifClient::onDiscoveryReadyRead() {
    auto* socket = qobject_cast<QUdpSocket*>(sender());
    if (!socket) {
        return;
    }
    while (socket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = socket->receiveDatagram();
        parseDiscoveryResponse(datagram.data(), datagram.senderAddress());
    }
}

void OnvifClient::onDiscoveryTimeout() {
    qDebug() << "OnvifClient: Discovery timeout, finishing...";
    stopDiscovery();
    emit discoveryFinished();
    qDebug() << "OnvifClient: discoveryFinished signal emitted";
}

QString OnvifClient::createWsDiscoveryProbe() const {
    QString messageId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    
    return QString(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<e:Envelope xmlns:e=\"http://www.w3.org/2003/05/soap-envelope\" "
        "xmlns:w=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
        "xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" "
        "xmlns:dn=\"http://www.onvif.org/ver10/network/wsdl\">"
        "<e:Header>"
        "<w:MessageID>uuid:%1</w:MessageID>"
        "<w:To>urn:schemas-xmlsoap-org:ws:2005:04:discovery</w:To>"
        "<w:Action>http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe</w:Action>"
        "</e:Header>"
        "<e:Body>"
        "<d:Probe>"
        "<d:Types>dn:NetworkVideoTransmitter</d:Types>"
        "</d:Probe>"
        "</e:Body>"
        "</e:Envelope>"
    ).arg(messageId);
}

void OnvifClient::parseDiscoveryResponse(const QByteArray& data, const QHostAddress& sender) {
    QXmlStreamReader xml(data);
    OnvifDevice device;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            QString name = xml.name().toString();

            if (name == "XAddrs") {
                QString xaddrs = xml.readElementText();
                QStringList addrs = xaddrs.split(' ', Qt::SkipEmptyParts);

                // IPv4 XAddr 을 우선한다. 일부 장치는 IPv6 링크로컬 주소를 먼저 광고한다.
                for (const QString& addr : addrs) {
                    QHostAddress host(QUrl(addr).host());
                    if (host.protocol() == QAbstractSocket::IPv4Protocol) {
                        device.xaddr = addr;
                        break;
                    }
                }
                if (device.xaddr.isEmpty() && !addrs.isEmpty()) {
                    device.xaddr = addrs.first();
                }
                device.serviceUrl = device.xaddr;
                device.address = QUrl(device.xaddr).host();
            } else if (name == "Scopes") {
                QString scopes = xml.readElementText();
                QStringList scopeList = scopes.split(' ', Qt::SkipEmptyParts);

                for (const QString& scope : scopeList) {
                    if (scope.contains("onvif://www.onvif.org/name/")) {
                        device.name = QUrl::fromPercentEncoding(scope.mid(scope.lastIndexOf('/') + 1).toUtf8());
                    } else if (scope.contains("onvif://www.onvif.org/hardware/")) {
                        device.model = QUrl::fromPercentEncoding(scope.mid(scope.lastIndexOf('/') + 1).toUtf8());
                    }
                }
            }
        }
    }

    QString senderIp = sender.toString();
    if (senderIp.startsWith("::ffff:")) {
        senderIp = senderIp.mid(7);
    }

    if (device.address.isEmpty()) {
        device.address = senderIp;
    }
    if (device.xaddr.isEmpty()) {
        if (device.address.isEmpty()) {
            return;
        }
        device.xaddr = QString("http://%1/onvif/device_service").arg(device.address);
        device.serviceUrl = device.xaddr;
    }

    // 프로브를 인터페이스마다, 그리고 두 번 보내므로 같은 장치가 여러 번 응답한다.
    if (m_discoveredAddresses.contains(device.address)) {
        return;
    }
    m_discoveredAddresses.insert(device.address);

    qInfo() << "OnvifClient: device" << device.address << device.name << device.model << "via" << senderIp;
    emit deviceDiscovered(device);
}

void OnvifClient::getCapabilities(const QString& deviceServiceUrl) {
    QString body = createGetCapabilitiesRequest();
    QString envelope = createSoapEnvelope(body);
    
    QUrl url(deviceServiceUrl);
    QNetworkRequest request{url};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/soap+xml; charset=utf-8");
    request.setRawHeader("SOAPAction", "\"http://www.onvif.org/ver10/device/wsdl/GetCapabilities\"");
    
    // HTTP Basic 을 선제적으로 보내면 비밀번호가 매 요청마다 평문으로 나간다.
    // 인증은 SOAP 본문의 WS-Security 다이제스트와, 401 챌린지에 답하는 authenticationRequired 핸들러로만 한다.
    
    QNetworkReply* reply = m_networkManager->post(request, envelope.toUtf8());
    reply->setProperty("requestType", "GetCapabilities");
    reply->setProperty("deviceUrl", deviceServiceUrl);
}

QString OnvifClient::createGetCapabilitiesRequest() const {
    return QString(
        "<tds:GetCapabilities xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\">"
        "<tds:Category>All</tds:Category>"
        "</tds:GetCapabilities>"
    );
}

void OnvifClient::getServices(const QString& deviceServiceUrl) {
    QString body = createGetServicesRequest();
    QString envelope = createSoapEnvelope(body);
    
    QUrl url(deviceServiceUrl);
    QNetworkRequest request{url};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/soap+xml; charset=utf-8");
    request.setRawHeader("SOAPAction", "\"http://www.onvif.org/ver10/device/wsdl/GetServices\"");
    
    // HTTP Basic 을 선제적으로 보내면 비밀번호가 매 요청마다 평문으로 나간다.
    // 인증은 SOAP 본문의 WS-Security 다이제스트와, 401 챌린지에 답하는 authenticationRequired 핸들러로만 한다.
    
    QNetworkReply* reply = m_networkManager->post(request, envelope.toUtf8());
    reply->setProperty("requestType", "GetServices");
    reply->setProperty("deviceUrl", deviceServiceUrl);
}

QString OnvifClient::createGetServicesRequest() const {
    return QString(
        "<tds:GetServices xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\">"
        "<tds:IncludeCapability>false</tds:IncludeCapability>"
        "</tds:GetServices>"
    );
}

void OnvifClient::getDeviceInformation(const QString& deviceServiceUrl) {
    QString body = createGetDeviceInformationRequest();
    QString envelope = createSoapEnvelope(body);
    
    QUrl url(deviceServiceUrl);
    QNetworkRequest request{url};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/soap+xml; charset=utf-8");
    request.setRawHeader("SOAPAction", "\"http://www.onvif.org/ver10/device/wsdl/GetDeviceInformation\"");
    
    // HTTP Basic 을 선제적으로 보내면 비밀번호가 매 요청마다 평문으로 나간다.
    // 인증은 SOAP 본문의 WS-Security 다이제스트와, 401 챌린지에 답하는 authenticationRequired 핸들러로만 한다.
    
    QNetworkReply* reply = m_networkManager->post(request, envelope.toUtf8());
    reply->setProperty("requestType", "GetDeviceInformation");
    reply->setProperty("deviceUrl", deviceServiceUrl);
}

QString OnvifClient::createGetDeviceInformationRequest() const {
    return QString(
        "<tds:GetDeviceInformation xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\"/>"
    );
}

void OnvifClient::parseDeviceInformationResponse(const QByteArray& data) {
    qDebug() << "Parsing device information response:" << data.left(1000);
    
    QXmlStreamReader xml(data);
    OnvifDeviceInfo info;
    
    while (!xml.atEnd()) {
        xml.readNext();
        
        if (xml.isStartElement()) {
            QString name = xml.name().toString();
            
            if (name == "Manufacturer") {
                info.manufacturer = xml.readElementText();
            } else if (name == "Model") {
                info.model = xml.readElementText();
            } else if (name == "FirmwareVersion") {
                info.firmwareVersion = xml.readElementText();
            } else if (name == "SerialNumber") {
                info.serialNumber = xml.readElementText();
            } else if (name == "HardwareId") {
                info.hardwareId = xml.readElementText();
            }
        }
    }
    
    qDebug() << "Device info - Manufacturer:" << info.manufacturer 
             << "Model:" << info.model 
             << "SerialNumber:" << info.serialNumber;
    
    emit deviceInformationReceived(info);
}

void OnvifClient::parseServicesResponse(const QByteArray& data) {
    qDebug() << "=== RAW GetServices Response ===";
    qDebug().noquote() << data;
    qDebug() << "=== END Response ===";
    
    m_capabilities = OnvifCapabilities();
    
    // Parse XML to find service XAddrs
    QXmlStreamReader xml(data);
    
    QString currentNamespace;
    QString currentXAddr;
    QString media2Url;

    // contains("media") 같은 느슨한 비교는 Media2(ver20) 나 deviceIO 에도 매칭되어
    // 엉뚱한 엔드포인트에 ver10 GetProfiles 를 보내게 된다. 네임스페이스를 정확히 비교한다.
    static const QString kMedia1  = QStringLiteral("http://www.onvif.org/ver10/media/wsdl");
    static const QString kMedia2  = QStringLiteral("http://www.onvif.org/ver20/media/wsdl");
    static const QString kPtz     = QStringLiteral("http://www.onvif.org/ver20/ptz/wsdl");
    static const QString kEvents  = QStringLiteral("http://www.onvif.org/ver10/events/wsdl");
    static const QString kImaging = QStringLiteral("http://www.onvif.org/ver20/imaging/wsdl");
    static const QString kDevice  = QStringLiteral("http://www.onvif.org/ver10/device/wsdl");
    
    while (!xml.atEnd()) {
        xml.readNext();
        
        if (xml.isStartElement()) {
            QString name = xml.name().toString();
            
            if (name == "Namespace") {
                currentNamespace = xml.readElementText();
            } else if (name == "XAddr") {
                currentXAddr = xml.readElementText();
                
                QString ns = currentNamespace.trimmed();
                if (ns == kMedia1) {
                    m_capabilities.mediaServiceUrl = currentXAddr;
                    qDebug() << "Found Media service:" << currentXAddr;
                } else if (ns == kMedia2) {
                    media2Url = currentXAddr;
                    qDebug() << "Found Media2 service:" << currentXAddr;
                } else if (ns == kPtz) {
                    m_capabilities.ptzServiceUrl = currentXAddr;
                    qDebug() << "Found PTZ service:" << currentXAddr;
                } else if (ns == kEvents) {
                    m_capabilities.eventsServiceUrl = currentXAddr;
                } else if (ns == kImaging) {
                    m_capabilities.imagingServiceUrl = currentXAddr;
                } else if (ns == kDevice) {
                    m_capabilities.deviceServiceUrl = currentXAddr;
                }
            }
        } else if (xml.isEndElement() && xml.name().toString() == "Service") {
            currentNamespace.clear();
            currentXAddr.clear();
        }
    }
    
    if (m_capabilities.mediaServiceUrl.isEmpty() && !media2Url.isEmpty()) {
        // Media2 만 광고하는 장치. ver10 GetProfiles 가 실패할 수 있지만 아무것도 없는 것보다는 낫다.
        qWarning() << "Device advertises only Media2; falling back to" << media2Url;
        m_capabilities.mediaServiceUrl = media2Url;
    }

    qDebug() << "Services parsed - Media:" << m_capabilities.mediaServiceUrl;
    
    emit capabilitiesReceived(m_capabilities);
}

void OnvifClient::parseCapabilitiesResponse(const QByteArray& data) {
    qDebug() << "=== RAW GetCapabilities Response ===";
    qDebug().noquote() << data;
    qDebug() << "=== END Response ===";
    
    QXmlStreamReader xml(data);
    m_capabilities = OnvifCapabilities();
    
    QString currentSection;
    
    while (!xml.atEnd()) {
        xml.readNext();
        
        if (xml.isStartElement()) {
            QString name = xml.name().toString();
            
            // Track which capability section we're in
            if (name == "Media" || name == "PTZ" || name == "Events" || 
                name == "Imaging" || name == "Device") {
                currentSection = name;
            }
            
            // Look for XAddr in any context
            if (name == "XAddr") {
                QString xaddr = xml.readElementText();
                qDebug() << "Found XAddr in section" << currentSection << ":" << xaddr;
                
                if (currentSection == "Media") {
                    m_capabilities.mediaServiceUrl = xaddr;
                } else if (currentSection == "PTZ") {
                    m_capabilities.ptzServiceUrl = xaddr;
                } else if (currentSection == "Events") {
                    m_capabilities.eventsServiceUrl = xaddr;
                } else if (currentSection == "Imaging") {
                    m_capabilities.imagingServiceUrl = xaddr;
                } else if (currentSection == "Device") {
                    m_capabilities.deviceServiceUrl = xaddr;
                }
            }
        } else if (xml.isEndElement()) {
            QString name = xml.name().toString();
            if (name == "Media" || name == "PTZ" || name == "Events" || 
                name == "Imaging" || name == "Device") {
                currentSection.clear();
            }
        }
    }
    
    qDebug() << "Capabilities parsed - Media:" << m_capabilities.mediaServiceUrl
             << "PTZ:" << m_capabilities.ptzServiceUrl;
    
    emit capabilitiesReceived(m_capabilities);
}

void OnvifClient::getProfiles(const QString& mediaServiceUrl) {
    m_currentMediaUrl = mediaServiceUrl;
    
    QString body = createGetProfilesRequest();
    QString envelope = createSoapEnvelope(body);
    
    QUrl url(mediaServiceUrl);
    QNetworkRequest request{url};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/soap+xml; charset=utf-8");
    request.setRawHeader("SOAPAction", "\"http://www.onvif.org/ver10/media/wsdl/GetProfiles\"");
    
    // HTTP Basic 을 선제적으로 보내면 비밀번호가 매 요청마다 평문으로 나간다.
    // 인증은 SOAP 본문의 WS-Security 다이제스트와, 401 챌린지에 답하는 authenticationRequired 핸들러로만 한다.
    
    QNetworkReply* reply = m_networkManager->post(request, envelope.toUtf8());
    reply->setProperty("requestType", "GetProfiles");
}

QString OnvifClient::createGetProfilesRequest() const {
    return QString("<trt:GetProfiles xmlns:trt=\"http://www.onvif.org/ver10/media/wsdl\"/>");
}

void OnvifClient::getStreamUri(const QString& mediaServiceUrl, const QString& profileToken) {
    QString body = createGetStreamUriRequest(profileToken);
    QString envelope = createSoapEnvelope(body);
    
    QUrl url(mediaServiceUrl);
    QNetworkRequest request{url};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/soap+xml; charset=utf-8");
    request.setRawHeader("SOAPAction", "\"http://www.onvif.org/ver10/media/wsdl/GetStreamUri\"");
    
    // HTTP Basic 을 선제적으로 보내면 비밀번호가 매 요청마다 평문으로 나간다.
    // 인증은 SOAP 본문의 WS-Security 다이제스트와, 401 챌린지에 답하는 authenticationRequired 핸들러로만 한다.
    
    QNetworkReply* reply = m_networkManager->post(request, envelope.toUtf8());
    reply->setProperty("requestType", "GetStreamUri");
    reply->setProperty("profileToken", profileToken);
}

QString OnvifClient::createGetStreamUriRequest(const QString& profileToken) const {
    return QString(
        "<trt:GetStreamUri xmlns:trt=\"http://www.onvif.org/ver10/media/wsdl\">"
        "<trt:StreamSetup>"
        "<tt:Stream xmlns:tt=\"http://www.onvif.org/ver10/schema\">RTP-Unicast</tt:Stream>"
        "<tt:Transport xmlns:tt=\"http://www.onvif.org/ver10/schema\">"
        "<tt:Protocol>RTSP</tt:Protocol>"
        "</tt:Transport>"
        "</trt:StreamSetup>"
        "<trt:ProfileToken>%1</trt:ProfileToken>"
        "</trt:GetStreamUri>"
    ).arg(profileToken);
}

void OnvifClient::ptzMove(const QString& ptzServiceUrl, const QString& profileToken,
                           PtzAction action, float speed) {
    QString body;
    if (action == PtzAction::Stop) {
        body = createPtzStopRequest(profileToken);
    } else {
        body = createPtzMoveRequest(profileToken, action, speed);
    }
    
    QString envelope = createSoapEnvelope(body);
    
    QUrl url(ptzServiceUrl);
    QNetworkRequest request{url};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/soap+xml; charset=utf-8");
    
    QString soapAction = (action == PtzAction::Stop) 
        ? "\"http://www.onvif.org/ver20/ptz/wsdl/Stop\""
        : "\"http://www.onvif.org/ver20/ptz/wsdl/ContinuousMove\"";
    request.setRawHeader("SOAPAction", soapAction.toUtf8());
    
    // HTTP Basic 을 선제적으로 보내면 비밀번호가 매 요청마다 평문으로 나간다.
    // 인증은 SOAP 본문의 WS-Security 다이제스트와, 401 챌린지에 답하는 authenticationRequired 핸들러로만 한다.
    
    QNetworkReply* reply = m_networkManager->post(request, envelope.toUtf8());
    reply->setProperty("requestType", "PTZ");
}

QString OnvifClient::createPtzMoveRequest(const QString& profileToken, PtzAction action, float speed) const {
    float panSpeed = 0;
    float tiltSpeed = 0;
    float zoomSpeed = 0;
    
    switch (action) {
        case PtzAction::Up:       tiltSpeed = speed; break;
        case PtzAction::Down:     tiltSpeed = -speed; break;
        case PtzAction::Left:     panSpeed = -speed; break;
        case PtzAction::Right:    panSpeed = speed; break;
        case PtzAction::UpLeft:   panSpeed = -speed; tiltSpeed = speed; break;
        case PtzAction::UpRight:  panSpeed = speed; tiltSpeed = speed; break;
        case PtzAction::DownLeft: panSpeed = -speed; tiltSpeed = -speed; break;
        case PtzAction::DownRight: panSpeed = speed; tiltSpeed = -speed; break;
        case PtzAction::ZoomIn:   zoomSpeed = speed; break;
        case PtzAction::ZoomOut:  zoomSpeed = -speed; break;
        default: break;
    }
    
    return QString(
        "<ptz:ContinuousMove xmlns:ptz=\"http://www.onvif.org/ver20/ptz/wsdl\">"
        "<ptz:ProfileToken>%1</ptz:ProfileToken>"
        "<ptz:Velocity>"
        "<tt:PanTilt xmlns:tt=\"http://www.onvif.org/ver10/schema\" x=\"%2\" y=\"%3\"/>"
        "<tt:Zoom xmlns:tt=\"http://www.onvif.org/ver10/schema\" x=\"%4\"/>"
        "</ptz:Velocity>"
        "</ptz:ContinuousMove>"
    ).arg(profileToken).arg(panSpeed).arg(tiltSpeed).arg(zoomSpeed);
}

QString OnvifClient::createPtzStopRequest(const QString& profileToken) const {
    return QString(
        "<ptz:Stop xmlns:ptz=\"http://www.onvif.org/ver20/ptz/wsdl\">"
        "<ptz:ProfileToken>%1</ptz:ProfileToken>"
        "<ptz:PanTilt>true</ptz:PanTilt>"
        "<ptz:Zoom>true</ptz:Zoom>"
        "</ptz:Stop>"
    ).arg(profileToken);
}

QString OnvifClient::createSoapEnvelope(const QString& body) const {
    QString securityHeader = createSecurityHeader();
    
    return QString(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\">"
        "<s:Header>%1</s:Header>"
        "<s:Body>%2</s:Body>"
        "</s:Envelope>"
    ).arg(securityHeader).arg(body);
}

QString OnvifClient::createSecurityHeader() const {
    if (m_username.isEmpty()) {
        return QString();
    }
    
    // Generate random nonce (20 bytes)
    QByteArray nonce(20, 0);
    for (int i = 0; i < 20; ++i) {
        nonce[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    }
    
    // Created timestamp in ISO 8601 format
    QString created = QDateTime::currentDateTimeUtc().toString("yyyy-MM-ddTHH:mm:ssZ");
    
    // PasswordDigest = Base64(SHA1(nonce + created + password))
    // Note: nonce is raw bytes, created and password are UTF-8
    QByteArray digestInput;
    digestInput.append(nonce);
    digestInput.append(created.toUtf8());
    digestInput.append(m_password.toUtf8());
    
    QByteArray digest = QCryptographicHash::hash(digestInput, QCryptographicHash::Sha1);
    
    QString nonceBase64 = QString::fromLatin1(nonce.toBase64());
    QString digestBase64 = QString::fromLatin1(digest.toBase64());
    
    return QString(
        "<wsse:Security xmlns:wsse=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\" "
        "xmlns:wsu=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\" "
        "s:mustUnderstand=\"1\">"
        "<wsse:UsernameToken>"
        "<wsse:Username>%1</wsse:Username>"
        "<wsse:Password Type=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest\">%2</wsse:Password>"
        "<wsse:Nonce EncodingType=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary\">%3</wsse:Nonce>"
        "<wsu:Created>%4</wsu:Created>"
        "</wsse:UsernameToken>"
        "</wsse:Security>"
    ).arg(m_username, digestBase64, nonceBase64, created);
}

void OnvifClient::onHttpFinished(QNetworkReply* reply) {
    QString requestType = reply->property("requestType").toString();
    QString requestUrl = reply->url().toString();
    
    QByteArray data = reply->readAll();
    
    if (reply->error() != QNetworkReply::NoError) {
        QString errorStr = reply->errorString();
        qDebug() << "HTTP error:" << errorStr;
        qDebug() << "Response:" << data;
        
        // 500/503 은 카메라가 잠시 바쁜 경우가 많아 재시도한다. 문자열 매칭 대신 상태 코드를 본다.
        int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        bool shouldRetry = (httpStatus == 500 || httpStatus == 503);
        
        if (shouldRetry) {
            int retries = m_retryCount.value(requestUrl, 0);
            if (retries < MAX_RETRIES) {
                m_retryCount[requestUrl] = retries + 1;
                qDebug() << "Retrying request" << requestType << "attempt" << (retries + 2) << "of" << (MAX_RETRIES + 1);
                
                // Schedule retry with delay
                QTimer::singleShot(RETRY_DELAY_MS * (retries + 1), this, [this, reply]() {
                    retryRequest(reply);
                });
                return;  // Don't delete reply yet, we need its properties
            }
        }
        
        m_retryCount.remove(requestUrl);
        emit error(QString("HTTP error: %1").arg(errorStr));
        reply->deleteLater();
        return;
    }
    
    // Success - clear retry count
    m_retryCount.remove(requestUrl);
    
    qDebug() << "Response for" << requestType << ":" << data.left(500);
    
    if (requestType == "GetCapabilities") {
        parseCapabilitiesResponse(data);
    } else if (requestType == "GetServices") {
        parseServicesResponse(data);
    } else if (requestType == "GetDeviceInformation") {
        parseDeviceInformationResponse(data);
    } else if (requestType == "GetProfiles") {
        parseProfilesResponse(data);
    } else if (requestType == "GetStreamUri") {
        QString profileToken = reply->property("profileToken").toString();
        parseStreamUriResponse(data, profileToken);
    } else if (requestType == "GetNetworkInterfaces") {
        parseNetworkInterfacesResponse(data);
    } else if (requestType == "SetNetworkInterfaces") {
        parseSetNetworkInterfacesResponse(data);
    } else if (requestType == "SetNetworkDefaultGateway") {
        emit networkSettingsChanged(false);
    } else if (requestType == "SystemReboot") {
        emit systemRebooting();
    }
    
    reply->deleteLater();
}

void OnvifClient::retryRequest(QNetworkReply* originalReply) {
    QString requestType = originalReply->property("requestType").toString();
    QString deviceUrl = originalReply->property("deviceUrl").toString();
    QString profileToken = originalReply->property("profileToken").toString();
    QUrl url = originalReply->url();
    
    originalReply->deleteLater();
    
    qDebug() << "Retrying" << requestType << "to" << url.toString();
    
    if (requestType == "GetServices") {
        getServices(deviceUrl.isEmpty() ? url.toString() : deviceUrl);
    } else if (requestType == "GetCapabilities") {
        getCapabilities(deviceUrl.isEmpty() ? url.toString() : deviceUrl);
    } else if (requestType == "GetProfiles") {
        getProfiles(url.toString());
    } else if (requestType == "GetStreamUri") {
        getStreamUri(url.toString(), profileToken);
    }
}

void OnvifClient::parseProfilesResponse(const QByteArray& data) {
    m_profiles.clear();
    QXmlStreamReader xml(data);
    
    OnvifProfile currentProfile;
    bool inProfile = false;
    bool inVideoEncoder = false;
    
    while (!xml.atEnd()) {
        xml.readNext();
        
        if (xml.isStartElement()) {
            QString name = xml.name().toString();
            
            if (name == "Profiles") {
                inProfile = true;
                currentProfile = OnvifProfile();
                currentProfile.token = xml.attributes().value("token").toString();
            } else if (inProfile) {
                if (name == "Name") {
                    currentProfile.name = xml.readElementText();
                } else if (name == "VideoEncoderConfiguration") {
                    inVideoEncoder = true;
                } else if (inVideoEncoder) {
                    if (name == "Encoding") {
                        currentProfile.encoding = xml.readElementText();
                    } else if (name == "Width") {
                        currentProfile.width = xml.readElementText().toInt();
                    } else if (name == "Height") {
                        currentProfile.height = xml.readElementText().toInt();
                    } else if (name == "FrameRateLimit") {
                        currentProfile.fps = xml.readElementText().toInt();
                    }
                }
            }
        } else if (xml.isEndElement()) {
            QString name = xml.name().toString();
            
            if (name == "Profiles") {
                if (!currentProfile.token.isEmpty()) {
                    m_profiles.append(currentProfile);
                }
                inProfile = false;
            } else if (name == "VideoEncoderConfiguration") {
                inVideoEncoder = false;
            }
        }
    }
    
    qDebug() << "Found" << m_profiles.size() << "profiles";
    
    emit profilesReceived(m_profiles);
}

void OnvifClient::parseStreamUriResponse(const QByteArray& data, const QString& profileToken) {
    QXmlStreamReader xml(data);
    
    while (!xml.atEnd()) {
        xml.readNext();
        
        if (xml.isStartElement() && xml.name().toString() == "Uri") {
            QString uri = xml.readElementText();
            m_currentStreamUri = uri;
            qDebug() << "Stream URI:" << uri;
            emit streamUriReceived(profileToken, uri);
            return;
        }
    }
    
    emit error("Failed to parse stream URI response");
}

// ============ Network Management ============

void OnvifClient::getNetworkInterfaces(const QString& deviceServiceUrl) {
    QString body = createGetNetworkInterfacesRequest();
    QString envelope = createSoapEnvelope(body);
    
    QUrl url(deviceServiceUrl);
    QNetworkRequest request{url};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/soap+xml; charset=utf-8");
    request.setRawHeader("SOAPAction", "\"http://www.onvif.org/ver10/device/wsdl/GetNetworkInterfaces\"");
    
    // HTTP Basic 을 선제적으로 보내면 비밀번호가 매 요청마다 평문으로 나간다.
    // 인증은 SOAP 본문의 WS-Security 다이제스트와, 401 챌린지에 답하는 authenticationRequired 핸들러로만 한다.
    
    QNetworkReply* reply = m_networkManager->post(request, envelope.toUtf8());
    reply->setProperty("requestType", "GetNetworkInterfaces");
    reply->setProperty("deviceUrl", deviceServiceUrl);
}

QString OnvifClient::createGetNetworkInterfacesRequest() const {
    return QString(
        "<tds:GetNetworkInterfaces xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\"/>"
    );
}

void OnvifClient::parseNetworkInterfacesResponse(const QByteArray& data) {
    qDebug() << "Parsing network interfaces response:" << data.left(2000);
    
    QList<NetworkInterface> interfaces;
    QXmlStreamReader xml(data);
    
    NetworkInterface currentIface;
    bool inInterface = false;
    bool inIPv4 = false;
    bool inManual = false;
    bool inFromDhcp = false;
    QString fromDhcpAddress;
    int fromDhcpPrefix = -1;
    
    while (!xml.atEnd()) {
        xml.readNext();
        
        if (xml.isStartElement()) {
            QString name = xml.name().toString();
            
            if (name == "NetworkInterfaces") {
                inInterface = true;
                currentIface = NetworkInterface();
                fromDhcpAddress.clear();
                fromDhcpPrefix = -1;
                // Get token attribute
                for (const auto& attr : xml.attributes()) {
                    if (attr.name().toString() == "token") {
                        currentIface.token = attr.value().toString();
                    }
                }
            } else if (inInterface) {
                if (name == "Enabled") {
                    currentIface.enabled = (xml.readElementText().toLower() == "true");
                } else if (name == "Info") {
                    // Skip to nested elements
                } else if (name == "Name") {
                    currentIface.name = xml.readElementText();
                } else if (name == "HwAddress") {
                    currentIface.macAddress = xml.readElementText();
                } else if (name == "IPv4") {
                    inIPv4 = true;
                } else if (inIPv4) {
                    if (name == "Enabled") {
                        // IPv4 enabled
                    } else if (name == "Config") {
                        // Configuration section
                    } else if (name == "DHCP") {
                        currentIface.dhcpEnabled = (xml.readElementText().toLower() == "true");
                    } else if (name == "Manual") {
                        inManual = true;
                    } else if (name == "FromDHCP") {
                        inFromDhcp = true;
                    } else if (inManual) {
                        if (name == "Address") {
                            currentIface.ipAddress = xml.readElementText();
                        } else if (name == "PrefixLength") {
                            currentIface.prefixLength = xml.readElementText().toInt();
                        }
                    } else if (inFromDhcp) {
                        // DHCP 로 받은 주소. 수동 주소가 없을 때 표시용으로 쓴다.
                        if (name == "Address") {
                            fromDhcpAddress = xml.readElementText();
                        } else if (name == "PrefixLength") {
                            fromDhcpPrefix = xml.readElementText().toInt();
                        }
                    }
                }
            }
        } else if (xml.isEndElement()) {
            QString name = xml.name().toString();
            
            if (name == "NetworkInterfaces") {
                inInterface = false;
                if (currentIface.ipAddress.isEmpty() && !fromDhcpAddress.isEmpty()) {
                    currentIface.ipAddress = fromDhcpAddress;
                    if (fromDhcpPrefix > 0) {
                        currentIface.prefixLength = fromDhcpPrefix;
                    }
                }
                if (!currentIface.token.isEmpty()) {
                    interfaces.append(currentIface);
                }
            } else if (name == "IPv4") {
                inIPv4 = false;
            } else if (name == "Manual") {
                inManual = false;
            } else if (name == "FromDHCP") {
                inFromDhcp = false;
            }
        }
    }
    
    qDebug() << "Found" << interfaces.size() << "network interfaces";
    for (const auto& iface : interfaces) {
        qDebug() << "  Interface:" << iface.token << "IP:" << iface.ipAddress 
                 << "DHCP:" << iface.dhcpEnabled;
    }
    
    emit networkInterfacesReceived(interfaces);
}

void OnvifClient::setNetworkInterfaces(const QString& deviceServiceUrl, const NetworkInterface& config) {
    QString body = createSetNetworkInterfacesRequest(config);
    QString envelope = createSoapEnvelope(body);
    
    qDebug() << "SetNetworkInterfaces request:" << envelope;
    
    QUrl url(deviceServiceUrl);
    QNetworkRequest request{url};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/soap+xml; charset=utf-8");
    request.setRawHeader("SOAPAction", "\"http://www.onvif.org/ver10/device/wsdl/SetNetworkInterfaces\"");
    
    // HTTP Basic 을 선제적으로 보내면 비밀번호가 매 요청마다 평문으로 나간다.
    // 인증은 SOAP 본문의 WS-Security 다이제스트와, 401 챌린지에 답하는 authenticationRequired 핸들러로만 한다.
    
    QNetworkReply* reply = m_networkManager->post(request, envelope.toUtf8());
    reply->setProperty("requestType", "SetNetworkInterfaces");
    reply->setProperty("deviceUrl", deviceServiceUrl);
}

QString OnvifClient::createSetNetworkInterfacesRequest(const NetworkInterface& config) const {
    QString ipv4Config;
    
    if (config.dhcpEnabled) {
        ipv4Config = QString(
            "<tt:IPv4>"
            "<tt:Enabled>true</tt:Enabled>"
            "<tt:DHCP>true</tt:DHCP>"
            "</tt:IPv4>"
        );
    } else {
        ipv4Config = QString(
            "<tt:IPv4>"
            "<tt:Enabled>true</tt:Enabled>"
            "<tt:DHCP>false</tt:DHCP>"
            "<tt:Manual>"
            "<tt:Address>%1</tt:Address>"
            "<tt:PrefixLength>%2</tt:PrefixLength>"
            "</tt:Manual>"
            "</tt:IPv4>"
        ).arg(config.ipAddress).arg(config.prefixLength);
    }
    
    return QString(
        "<tds:SetNetworkInterfaces xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\" "
        "xmlns:tt=\"http://www.onvif.org/ver10/schema\">"
        "<tds:InterfaceToken>%1</tds:InterfaceToken>"
        "<tds:NetworkInterface>"
        "<tt:Enabled>%2</tt:Enabled>"
        "%3"
        "</tds:NetworkInterface>"
        "</tds:SetNetworkInterfaces>"
    ).arg(config.token)
     .arg(config.enabled ? "true" : "false")
     .arg(ipv4Config);
}

void OnvifClient::parseSetNetworkInterfacesResponse(const QByteArray& data) {
    qDebug() << "SetNetworkInterfaces response:" << data;
    
    QXmlStreamReader xml(data);
    bool rebootNeeded = false;
    
    while (!xml.atEnd()) {
        xml.readNext();
        
        if (xml.isStartElement()) {
            QString name = xml.name().toString();
            if (name == "RebootNeeded") {
                rebootNeeded = (xml.readElementText().toLower() == "true");
            }
        }
    }
    
    emit networkSettingsChanged(rebootNeeded);
}

void OnvifClient::getNetworkDefaultGateway(const QString& deviceServiceUrl) {
    QString body = "<tds:GetNetworkDefaultGateway xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\"/>";
    QString envelope = createSoapEnvelope(body);
    
    QUrl url(deviceServiceUrl);
    QNetworkRequest request{url};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/soap+xml; charset=utf-8");
    request.setRawHeader("SOAPAction", "\"http://www.onvif.org/ver10/device/wsdl/GetNetworkDefaultGateway\"");
    
    // HTTP Basic 을 선제적으로 보내면 비밀번호가 매 요청마다 평문으로 나간다.
    // 인증은 SOAP 본문의 WS-Security 다이제스트와, 401 챌린지에 답하는 authenticationRequired 핸들러로만 한다.
    
    QNetworkReply* reply = m_networkManager->post(request, envelope.toUtf8());
    reply->setProperty("requestType", "GetNetworkDefaultGateway");
}

void OnvifClient::setNetworkDefaultGateway(const QString& deviceServiceUrl, const QString& gateway) {
    QString body = createSetNetworkDefaultGatewayRequest(gateway);
    QString envelope = createSoapEnvelope(body);
    
    QUrl url(deviceServiceUrl);
    QNetworkRequest request{url};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/soap+xml; charset=utf-8");
    request.setRawHeader("SOAPAction", "\"http://www.onvif.org/ver10/device/wsdl/SetNetworkDefaultGateway\"");
    
    // HTTP Basic 을 선제적으로 보내면 비밀번호가 매 요청마다 평문으로 나간다.
    // 인증은 SOAP 본문의 WS-Security 다이제스트와, 401 챌린지에 답하는 authenticationRequired 핸들러로만 한다.
    
    QNetworkReply* reply = m_networkManager->post(request, envelope.toUtf8());
    reply->setProperty("requestType", "SetNetworkDefaultGateway");
}

QString OnvifClient::createSetNetworkDefaultGatewayRequest(const QString& gateway) const {
    return QString(
        "<tds:SetNetworkDefaultGateway xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\">"
        "<tds:IPv4Address>%1</tds:IPv4Address>"
        "</tds:SetNetworkDefaultGateway>"
    ).arg(gateway);
}

void OnvifClient::systemReboot(const QString& deviceServiceUrl) {
    QString body = createSystemRebootRequest();
    QString envelope = createSoapEnvelope(body);
    
    QUrl url(deviceServiceUrl);
    QNetworkRequest request{url};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/soap+xml; charset=utf-8");
    request.setRawHeader("SOAPAction", "\"http://www.onvif.org/ver10/device/wsdl/SystemReboot\"");
    
    // HTTP Basic 을 선제적으로 보내면 비밀번호가 매 요청마다 평문으로 나간다.
    // 인증은 SOAP 본문의 WS-Security 다이제스트와, 401 챌린지에 답하는 authenticationRequired 핸들러로만 한다.
    
    QNetworkReply* reply = m_networkManager->post(request, envelope.toUtf8());
    reply->setProperty("requestType", "SystemReboot");
}

QString OnvifClient::createSystemRebootRequest() const {
    return QString(
        "<tds:SystemReboot xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\"/>"
    );
}

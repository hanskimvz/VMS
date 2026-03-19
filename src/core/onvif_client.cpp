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

void OnvifClient::discover(int timeout) {
    qDebug() << "OnvifClient: Starting discovery with timeout" << timeout << "ms";
    
    if (m_discoverySocket) {
        stopDiscovery();
    }
    
    m_discoverySocket = new QUdpSocket(this);
    
    if (!m_discoverySocket->bind(QHostAddress::AnyIPv4, 0)) {
        qDebug() << "OnvifClient: Failed to bind discovery socket";
        emit error("Failed to bind discovery socket");
        delete m_discoverySocket;
        m_discoverySocket = nullptr;
        return;
    }
    
    connect(m_discoverySocket, &QUdpSocket::readyRead,
            this, &OnvifClient::onDiscoveryReadyRead);
    
    QString probeMsg = createWsDiscoveryProbe();
    QByteArray data = probeMsg.toUtf8();
    
    qint64 sent = m_discoverySocket->writeDatagram(data, QHostAddress(WS_DISCOVERY_ADDRESS), WS_DISCOVERY_PORT);
    qDebug() << "OnvifClient: Sent WS-Discovery probe," << sent << "bytes";
    
    m_discoveryTimer->start(timeout);
    qDebug() << "OnvifClient: Discovery timer started";
}

void OnvifClient::stopDiscovery() {
    m_discoveryTimer->stop();
    
    if (m_discoverySocket) {
        m_discoverySocket->close();
        delete m_discoverySocket;
        m_discoverySocket = nullptr;
    }
}

void OnvifClient::onDiscoveryReadyRead() {
    while (m_discoverySocket && m_discoverySocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_discoverySocket->pendingDatagramSize());
        
        QHostAddress sender;
        quint16 senderPort;
        
        m_discoverySocket->readDatagram(datagram.data(), datagram.size(),
                                         &sender, &senderPort);
        
        parseDiscoveryResponse(datagram);
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

void OnvifClient::parseDiscoveryResponse(const QByteArray& data) {
    QXmlStreamReader xml(data);
    OnvifDevice device;
    
    while (!xml.atEnd()) {
        xml.readNext();
        
        if (xml.isStartElement()) {
            QString name = xml.name().toString();
            
            if (name == "XAddrs") {
                QString xaddrs = xml.readElementText();
                QStringList addrs = xaddrs.split(' ', Qt::SkipEmptyParts);
                if (!addrs.isEmpty()) {
                    device.xaddr = addrs.first();
                    device.serviceUrl = device.xaddr;
                    QUrl url(device.xaddr);
                    device.address = url.host();
                }
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
    
    if (!device.xaddr.isEmpty()) {
        emit deviceDiscovered(device);
    }
}

void OnvifClient::getCapabilities(const QString& deviceServiceUrl) {
    QString body = createGetCapabilitiesRequest();
    QString envelope = createSoapEnvelope(body);
    
    QUrl url(deviceServiceUrl);
    QNetworkRequest request{url};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/soap+xml; charset=utf-8");
    request.setRawHeader("SOAPAction", "\"http://www.onvif.org/ver10/device/wsdl/GetCapabilities\"");
    
    // Add HTTP Basic Auth as fallback
    if (!m_username.isEmpty()) {
        QString credentials = QString("%1:%2").arg(m_username, m_password);
        QByteArray base64Credentials = credentials.toUtf8().toBase64();
        request.setRawHeader("Authorization", "Basic " + base64Credentials);
    }
    
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
    
    if (!m_username.isEmpty()) {
        QString credentials = QString("%1:%2").arg(m_username, m_password);
        QByteArray base64Credentials = credentials.toUtf8().toBase64();
        request.setRawHeader("Authorization", "Basic " + base64Credentials);
    }
    
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
    
    if (!m_username.isEmpty()) {
        QString credentials = QString("%1:%2").arg(m_username, m_password);
        QByteArray base64Credentials = credentials.toUtf8().toBase64();
        request.setRawHeader("Authorization", "Basic " + base64Credentials);
    }
    
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
    
    while (!xml.atEnd()) {
        xml.readNext();
        
        if (xml.isStartElement()) {
            QString name = xml.name().toString();
            
            if (name == "Namespace") {
                currentNamespace = xml.readElementText();
            } else if (name == "XAddr") {
                currentXAddr = xml.readElementText();
                
                // Map namespace to service
                if (currentNamespace.contains("media")) {
                    m_capabilities.mediaServiceUrl = currentXAddr;
                    qDebug() << "Found Media service:" << currentXAddr;
                } else if (currentNamespace.contains("ptz")) {
                    m_capabilities.ptzServiceUrl = currentXAddr;
                    qDebug() << "Found PTZ service:" << currentXAddr;
                } else if (currentNamespace.contains("event")) {
                    m_capabilities.eventsServiceUrl = currentXAddr;
                } else if (currentNamespace.contains("imaging")) {
                    m_capabilities.imagingServiceUrl = currentXAddr;
                } else if (currentNamespace.contains("device")) {
                    m_capabilities.deviceServiceUrl = currentXAddr;
                }
            }
        } else if (xml.isEndElement() && xml.name().toString() == "Service") {
            currentNamespace.clear();
            currentXAddr.clear();
        }
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
    
    // Add HTTP Basic Auth
    if (!m_username.isEmpty()) {
        QString credentials = QString("%1:%2").arg(m_username, m_password);
        QByteArray base64Credentials = credentials.toUtf8().toBase64();
        request.setRawHeader("Authorization", "Basic " + base64Credentials);
    }
    
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
    
    // Add HTTP Basic Auth
    if (!m_username.isEmpty()) {
        QString credentials = QString("%1:%2").arg(m_username, m_password);
        QByteArray base64Credentials = credentials.toUtf8().toBase64();
        request.setRawHeader("Authorization", "Basic " + base64Credentials);
    }
    
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
    
    // Add HTTP Basic Auth
    if (!m_username.isEmpty()) {
        QString credentials = QString("%1:%2").arg(m_username, m_password);
        QByteArray base64Credentials = credentials.toUtf8().toBase64();
        request.setRawHeader("Authorization", "Basic " + base64Credentials);
    }
    
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
        
        // Check if we should retry (500, 503 errors)
        bool shouldRetry = errorStr.contains("500") || 
                           errorStr.contains("503") ||
                           errorStr.contains("Internal Server Error") ||
                           errorStr.contains("Service Not Available") ||
                           errorStr.contains("Service Unavailable");
        
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
    
    if (!m_username.isEmpty()) {
        QString credentials = QString("%1:%2").arg(m_username, m_password);
        QByteArray base64Credentials = credentials.toUtf8().toBase64();
        request.setRawHeader("Authorization", "Basic " + base64Credentials);
    }
    
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
    
    while (!xml.atEnd()) {
        xml.readNext();
        
        if (xml.isStartElement()) {
            QString name = xml.name().toString();
            
            if (name == "NetworkInterfaces") {
                inInterface = true;
                currentIface = NetworkInterface();
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
                    } else if (inManual) {
                        if (name == "Address") {
                            currentIface.ipAddress = xml.readElementText();
                        } else if (name == "PrefixLength") {
                            currentIface.prefixLength = xml.readElementText().toInt();
                        }
                    } else if (name == "FromDHCP") {
                        // DHCP assigned address
                        // Read nested Address if we don't have manual
                    }
                }
            }
        } else if (xml.isEndElement()) {
            QString name = xml.name().toString();
            
            if (name == "NetworkInterfaces") {
                inInterface = false;
                if (!currentIface.token.isEmpty()) {
                    interfaces.append(currentIface);
                }
            } else if (name == "IPv4") {
                inIPv4 = false;
            } else if (name == "Manual") {
                inManual = false;
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
    
    if (!m_username.isEmpty()) {
        QString credentials = QString("%1:%2").arg(m_username, m_password);
        QByteArray base64Credentials = credentials.toUtf8().toBase64();
        request.setRawHeader("Authorization", "Basic " + base64Credentials);
    }
    
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
    
    if (!m_username.isEmpty()) {
        QString credentials = QString("%1:%2").arg(m_username, m_password);
        QByteArray base64Credentials = credentials.toUtf8().toBase64();
        request.setRawHeader("Authorization", "Basic " + base64Credentials);
    }
    
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
    
    if (!m_username.isEmpty()) {
        QString credentials = QString("%1:%2").arg(m_username, m_password);
        QByteArray base64Credentials = credentials.toUtf8().toBase64();
        request.setRawHeader("Authorization", "Basic " + base64Credentials);
    }
    
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
    
    if (!m_username.isEmpty()) {
        QString credentials = QString("%1:%2").arg(m_username, m_password);
        QByteArray base64Credentials = credentials.toUtf8().toBase64();
        request.setRawHeader("Authorization", "Basic " + base64Credentials);
    }
    
    QNetworkReply* reply = m_networkManager->post(request, envelope.toUtf8());
    reply->setProperty("requestType", "SystemReboot");
}

QString OnvifClient::createSystemRebootRequest() const {
    return QString(
        "<tds:SystemReboot xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\"/>"
    );
}

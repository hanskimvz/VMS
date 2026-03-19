#include "device_discovery.h"
#include <QNetworkDatagram>
#include <QDebug>
#include <QRegularExpression>
#include <QUrl>
#include <QUuid>

DeviceDiscovery::DeviceDiscovery(QObject* parent)
    : QObject(parent)
{
    m_discoveryTimer = new QTimer(this);
    m_discoveryTimer->setSingleShot(true);
    connect(m_discoveryTimer, &QTimer::timeout, this, &DeviceDiscovery::onDiscoveryTimeout);
}

DeviceDiscovery::~DeviceDiscovery() {
    stopDiscovery();
}

void DeviceDiscovery::startDiscovery(int timeout) {
    if (m_discovering) {
        stopDiscovery();
    }
    
    m_discovering = true;
    m_discoveredIPs.clear();
    
    startMdnsDiscovery();
    startSsdpDiscovery();
    startWsDiscovery();
    
    m_discoveryTimer->start(timeout);
}

void DeviceDiscovery::stopDiscovery() {
    m_discoveryTimer->stop();
    m_discovering = false;
    
    if (m_mdnsSocket) {
        m_mdnsSocket->close();
        delete m_mdnsSocket;
        m_mdnsSocket = nullptr;
    }
    
    if (m_ssdpSocket) {
        m_ssdpSocket->close();
        delete m_ssdpSocket;
        m_ssdpSocket = nullptr;
    }
    
    if (m_wsDiscoverySocket) {
        m_wsDiscoverySocket->close();
        delete m_wsDiscoverySocket;
        m_wsDiscoverySocket = nullptr;
    }
}

void DeviceDiscovery::startMdnsDiscovery() {
    m_mdnsSocket = new QUdpSocket(this);
    
    m_mdnsSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, QVariant(255));
    
    if (!m_mdnsSocket->bind(QHostAddress::AnyIPv4, MDNS_PORT, 
                            QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        qDebug() << "mDNS: Failed to bind socket";
        delete m_mdnsSocket;
        m_mdnsSocket = nullptr;
        return;
    }
    
    m_mdnsSocket->joinMulticastGroup(QHostAddress(MDNS_ADDRESS));
    
    connect(m_mdnsSocket, &QUdpSocket::readyRead, this, &DeviceDiscovery::onMdnsReadyRead);
    
    // Query for RTSP services (_rtsp._tcp.local)
    // mDNS query format (simplified)
    QByteArray query;
    
    // Transaction ID
    query.append('\x00'); query.append('\x00');
    // Flags (standard query)
    query.append('\x00'); query.append('\x00');
    // Questions: 3
    query.append('\x00'); query.append('\x03');
    // Answer RRs, Authority RRs, Additional RRs: 0
    query.append('\x00'); query.append('\x00');
    query.append('\x00'); query.append('\x00');
    query.append('\x00'); query.append('\x00');
    
    // Query 1: _rtsp._tcp.local
    auto appendLabel = [&query](const QString& label) {
        QByteArray data = label.toUtf8();
        query.append(static_cast<char>(data.length()));
        query.append(data);
    };
    
    appendLabel("_rtsp");
    appendLabel("_tcp");
    appendLabel("local");
    query.append('\x00');  // End of name
    query.append('\x00'); query.append('\x0c');  // Type: PTR
    query.append('\x00'); query.append('\x01');  // Class: IN
    
    // Query 2: _axis-video._tcp.local (Axis cameras)
    appendLabel("_axis-video");
    appendLabel("_tcp");
    appendLabel("local");
    query.append('\x00');
    query.append('\x00'); query.append('\x0c');
    query.append('\x00'); query.append('\x01');
    
    // Query 3: _http._tcp.local (web interfaces)
    appendLabel("_http");
    appendLabel("_tcp");
    appendLabel("local");
    query.append('\x00');
    query.append('\x00'); query.append('\x0c');
    query.append('\x00'); query.append('\x01');
    
    m_mdnsSocket->writeDatagram(query, QHostAddress(MDNS_ADDRESS), MDNS_PORT);
    qDebug() << "mDNS: Sent discovery query";
}

void DeviceDiscovery::startSsdpDiscovery() {
    m_ssdpSocket = new QUdpSocket(this);
    
    if (!m_ssdpSocket->bind(QHostAddress::AnyIPv4, 0)) {
        qDebug() << "SSDP: Failed to bind socket";
        delete m_ssdpSocket;
        m_ssdpSocket = nullptr;
        return;
    }
    
    connect(m_ssdpSocket, &QUdpSocket::readyRead, this, &DeviceDiscovery::onSsdpReadyRead);
    
    // M-SEARCH request for UPnP devices
    QByteArray searchRequest = 
        "M-SEARCH * HTTP/1.1\r\n"
        "HOST: 239.255.255.250:1900\r\n"
        "MAN: \"ssdp:discover\"\r\n"
        "MX: 3\r\n"
        "ST: ssdp:all\r\n"
        "\r\n";
    
    m_ssdpSocket->writeDatagram(searchRequest, QHostAddress(SSDP_ADDRESS), SSDP_PORT);
    
    // Also search for specific device types
    QByteArray mediaServerSearch = 
        "M-SEARCH * HTTP/1.1\r\n"
        "HOST: 239.255.255.250:1900\r\n"
        "MAN: \"ssdp:discover\"\r\n"
        "MX: 3\r\n"
        "ST: urn:schemas-upnp-org:device:MediaServer:1\r\n"
        "\r\n";
    
    m_ssdpSocket->writeDatagram(mediaServerSearch, QHostAddress(SSDP_ADDRESS), SSDP_PORT);
    
    QByteArray basicDeviceSearch = 
        "M-SEARCH * HTTP/1.1\r\n"
        "HOST: 239.255.255.250:1900\r\n"
        "MAN: \"ssdp:discover\"\r\n"
        "MX: 3\r\n"
        "ST: urn:schemas-upnp-org:device:Basic:1\r\n"
        "\r\n";
    
    m_ssdpSocket->writeDatagram(basicDeviceSearch, QHostAddress(SSDP_ADDRESS), SSDP_PORT);
    
    qDebug() << "SSDP: Sent M-SEARCH requests";
}

void DeviceDiscovery::startWsDiscovery() {
    m_wsDiscoverySocket = new QUdpSocket(this);
    
    if (!m_wsDiscoverySocket->bind(QHostAddress::AnyIPv4, 0)) {
        qDebug() << "WS-Discovery: Failed to bind socket";
        delete m_wsDiscoverySocket;
        m_wsDiscoverySocket = nullptr;
        return;
    }
    
    connect(m_wsDiscoverySocket, &QUdpSocket::readyRead, this, &DeviceDiscovery::onWsDiscoveryReadyRead);
    
    // WS-Discovery Probe message for ONVIF devices
    QString messageId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    
    QString probeMessage = QString(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<e:Envelope xmlns:e=\"http://www.w3.org/2003/05/soap-envelope\" "
        "xmlns:w=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
        "xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" "
        "xmlns:dn=\"http://www.onvif.org/ver10/network/wsdl\">"
        "<e:Header>"
        "<w:MessageID>uuid:%1</w:MessageID>"
        "<w:To e:mustUnderstand=\"true\">urn:schemas-xmlsoap-org:ws:2005:04:discovery</w:To>"
        "<w:Action e:mustUnderstand=\"true\">http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe</w:Action>"
        "</e:Header>"
        "<e:Body>"
        "<d:Probe>"
        "<d:Types>dn:NetworkVideoTransmitter</d:Types>"
        "</d:Probe>"
        "</e:Body>"
        "</e:Envelope>"
    ).arg(messageId);
    
    QByteArray data = probeMessage.toUtf8();
    m_wsDiscoverySocket->writeDatagram(data, QHostAddress(WS_DISCOVERY_ADDRESS), WS_DISCOVERY_PORT);
    
    qDebug() << "WS-Discovery: Sent ONVIF Probe message";
}

void DeviceDiscovery::onWsDiscoveryReadyRead() {
    while (m_wsDiscoverySocket && m_wsDiscoverySocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_wsDiscoverySocket->receiveDatagram();
        parseWsDiscoveryResponse(datagram.data(), datagram.senderAddress());
    }
}

void DeviceDiscovery::parseWsDiscoveryResponse(const QByteArray& data, const QHostAddress& sender) {
    QString response = QString::fromUtf8(data);
    
    // Check if it's a ProbeMatch response
    if (!response.contains("ProbeMatch", Qt::CaseInsensitive)) {
        return;
    }
    
    QString senderIP = sender.toString();
    if (senderIP.startsWith("::ffff:")) {
        senderIP = senderIP.mid(7);
    }
    
    // Skip if already discovered
    if (m_discoveredIPs.contains(senderIP)) return;
    
    m_discoveredIPs.insert(senderIP);
    
    DiscoveredDevice device;
    device.ip = senderIP;
    device.discoveryType = "ONVIF";
    device.port = 80;  // Default ONVIF port
    
    // Extract XAddrs (service address)
    QRegularExpression xaddrsRegex("<[^:]*:?XAddrs>([^<]+)</[^:]*:?XAddrs>", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = xaddrsRegex.match(response);
    if (match.hasMatch()) {
        device.serviceUrl = match.captured(1).trimmed().split(" ").first();
        
        // Extract port from service URL
        QUrl url(device.serviceUrl);
        if (url.port() > 0) {
            device.port = url.port();
        }
    }
    
    // Extract Scopes for device info
    QRegularExpression scopesRegex("<[^:]*:?Scopes>([^<]+)</[^:]*:?Scopes>", QRegularExpression::CaseInsensitiveOption);
    match = scopesRegex.match(response);
    if (match.hasMatch()) {
        QString scopes = match.captured(1);
        
        // Extract name from scope
        QRegularExpression nameRegex("onvif://www\\.onvif\\.org/name/([^\\s]+)");
        QRegularExpressionMatch nameMatch = nameRegex.match(scopes);
        if (nameMatch.hasMatch()) {
            device.name = QUrl::fromPercentEncoding(nameMatch.captured(1).toUtf8());
        }
        
        // Extract hardware from scope
        QRegularExpression hwRegex("onvif://www\\.onvif\\.org/hardware/([^\\s]+)");
        QRegularExpressionMatch hwMatch = hwRegex.match(scopes);
        if (hwMatch.hasMatch()) {
            device.model = QUrl::fromPercentEncoding(hwMatch.captured(1).toUtf8());
        }
    }
    
    if (device.name.isEmpty()) {
        device.name = QString("ONVIF Camera (%1)").arg(senderIP);
    }
    
    qDebug() << "WS-Discovery: Found ONVIF device at" << senderIP << "Service:" << device.serviceUrl;
    emit deviceDiscovered(device);
}

void DeviceDiscovery::onMdnsReadyRead() {
    while (m_mdnsSocket && m_mdnsSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_mdnsSocket->receiveDatagram();
        parseMdnsResponse(datagram.data(), datagram.senderAddress());
    }
}

void DeviceDiscovery::onSsdpReadyRead() {
    while (m_ssdpSocket && m_ssdpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_ssdpSocket->receiveDatagram();
        parseSsdpResponse(datagram.data(), datagram.senderAddress());
    }
}

void DeviceDiscovery::onDiscoveryTimeout() {
    qDebug() << "DeviceDiscovery: Timeout, finishing...";
    stopDiscovery();
    emit discoveryFinished();
    qDebug() << "DeviceDiscovery: discoveryFinished signal emitted";
}

void DeviceDiscovery::parseMdnsResponse(const QByteArray& data, const QHostAddress& sender) {
    if (data.size() < 12) return;
    
    QString senderIP = sender.toString();
    if (senderIP.startsWith("::ffff:")) {
        senderIP = senderIP.mid(7);
    }
    
    // Skip if already discovered
    if (m_discoveredIPs.contains(senderIP)) return;
    
    // Check if this is a response (QR bit set)
    quint16 flags = (static_cast<quint8>(data[2]) << 8) | static_cast<quint8>(data[3]);
    if (!(flags & 0x8000)) return;  // Not a response
    
    // Parse answer count
    quint16 answerCount = (static_cast<quint8>(data[6]) << 8) | static_cast<quint8>(data[7]);
    if (answerCount == 0) return;
    
    QString dataStr = QString::fromUtf8(data);
    
    // Exclude non-camera devices
    bool isExcluded = dataStr.contains("_printer", Qt::CaseInsensitive) ||
                      dataStr.contains("_scanner", Qt::CaseInsensitive) ||
                      dataStr.contains("_airplay", Qt::CaseInsensitive) ||
                      dataStr.contains("_raop", Qt::CaseInsensitive) ||
                      dataStr.contains("_spotify", Qt::CaseInsensitive) ||
                      dataStr.contains("_googlecast", Qt::CaseInsensitive) ||
                      dataStr.contains("_smb", Qt::CaseInsensitive) ||
                      dataStr.contains("_afpovertcp", Qt::CaseInsensitive) ||
                      dataStr.contains("_homekit", Qt::CaseInsensitive) ||
                      dataStr.contains("_hap", Qt::CaseInsensitive);
    
    if (isExcluded) return;
    
    // Look for camera-related services in the response
    bool isCamera = dataStr.contains("_rtsp", Qt::CaseInsensitive) ||
                    dataStr.contains("_axis-video", Qt::CaseInsensitive) ||
                    dataStr.contains("camera", Qt::CaseInsensitive) ||
                    dataStr.contains("hikvision", Qt::CaseInsensitive) ||
                    dataStr.contains("dahua", Qt::CaseInsensitive) ||
                    dataStr.contains("foscam", Qt::CaseInsensitive) ||
                    dataStr.contains("reolink", Qt::CaseInsensitive) ||
                    dataStr.contains("amcrest", Qt::CaseInsensitive) ||
                    dataStr.contains("ipcam", Qt::CaseInsensitive) ||
                    dataStr.contains("webcam", Qt::CaseInsensitive) ||
                    dataStr.contains("nvr", Qt::CaseInsensitive) ||
                    dataStr.contains("dvr", Qt::CaseInsensitive);
    
    if (!isCamera) return;
    
    m_discoveredIPs.insert(senderIP);
    
    DiscoveredDevice device;
    device.ip = senderIP;
    device.discoveryType = "mDNS";
    device.port = 554;  // Default RTSP port
    device.name = QString("mDNS Camera (%1)").arg(senderIP);
    
    qDebug() << "mDNS: Found camera at" << senderIP;
    emit deviceDiscovered(device);
}

void DeviceDiscovery::parseSsdpResponse(const QByteArray& data, const QHostAddress& sender) {
    QString response = QString::fromUtf8(data);
    
    // Check if it's an HTTP response or notification
    if (!response.startsWith("HTTP/1.1 200") && !response.startsWith("NOTIFY")) {
        return;
    }
    
    QString senderIP = sender.toString();
    if (senderIP.startsWith("::ffff:")) {
        senderIP = senderIP.mid(7);
    }
    
    // Skip if already discovered
    if (m_discoveredIPs.contains(senderIP)) return;
    
    // Exclude non-camera devices (routers, gateways, printers, etc.)
    bool isExcluded = response.contains("InternetGatewayDevice", Qt::CaseInsensitive) ||
                      response.contains("WANDevice", Qt::CaseInsensitive) ||
                      response.contains("WANConnection", Qt::CaseInsensitive) ||
                      response.contains("linuxigd", Qt::CaseInsensitive) ||
                      response.contains("router", Qt::CaseInsensitive) ||
                      response.contains("gateway", Qt::CaseInsensitive) ||
                      response.contains("printer", Qt::CaseInsensitive) ||
                      response.contains("scanner", Qt::CaseInsensitive) ||
                      response.contains("ContentDirectory", Qt::CaseInsensitive) ||
                      response.contains("RenderingControl", Qt::CaseInsensitive) ||
                      response.contains("AVTransport", Qt::CaseInsensitive) ||
                      response.contains("MediaRenderer", Qt::CaseInsensitive) ||
                      response.contains("DLNA", Qt::CaseInsensitive) ||
                      response.contains("Sonos", Qt::CaseInsensitive) ||
                      response.contains("Roku", Qt::CaseInsensitive) ||
                      response.contains("Chromecast", Qt::CaseInsensitive) ||
                      response.contains("SmartTV", Qt::CaseInsensitive) ||
                      response.contains("Xbox", Qt::CaseInsensitive) ||
                      response.contains("PlayStation", Qt::CaseInsensitive);
    
    if (isExcluded) return;
    
    // Check if it's likely a camera/surveillance device
    bool isCamera = response.contains("camera", Qt::CaseInsensitive) ||
                    response.contains("IPC", Qt::CaseInsensitive) ||
                    response.contains("NVR", Qt::CaseInsensitive) ||
                    response.contains("DVR", Qt::CaseInsensitive) ||
                    response.contains("surveillance", Qt::CaseInsensitive) ||
                    response.contains("hikvision", Qt::CaseInsensitive) ||
                    response.contains("dahua", Qt::CaseInsensitive) ||
                    response.contains("axis", Qt::CaseInsensitive) ||
                    response.contains("onvif", Qt::CaseInsensitive) ||
                    response.contains("foscam", Qt::CaseInsensitive) ||
                    response.contains("reolink", Qt::CaseInsensitive) ||
                    response.contains("amcrest", Qt::CaseInsensitive) ||
                    response.contains("vivotek", Qt::CaseInsensitive) ||
                    response.contains("hanwha", Qt::CaseInsensitive) ||
                    response.contains("uniview", Qt::CaseInsensitive) ||
                    response.contains("geovision", Qt::CaseInsensitive);
    
    if (!isCamera) return;
    
    m_discoveredIPs.insert(senderIP);
    
    DiscoveredDevice device;
    device.ip = senderIP;
    device.discoveryType = "UPnP";
    device.port = 554;  // Default RTSP port
    
    // Extract LOCATION header for service URL
    QRegularExpression locationRegex("LOCATION:\\s*(.+)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = locationRegex.match(response);
    if (match.hasMatch()) {
        device.serviceUrl = match.captured(1).trimmed();
        
        // Try to extract port from location URL
        QUrl url(device.serviceUrl);
        if (url.port() > 0) {
            device.port = url.port();
        }
    }
    
    // Extract SERVER header for device info
    QRegularExpression serverRegex("SERVER:\\s*(.+)", QRegularExpression::CaseInsensitiveOption);
    match = serverRegex.match(response);
    if (match.hasMatch()) {
        device.model = match.captured(1).trimmed();
    }
    
    // Extract USN for unique identification
    QRegularExpression usnRegex("USN:\\s*(.+)", QRegularExpression::CaseInsensitiveOption);
    match = usnRegex.match(response);
    if (match.hasMatch()) {
        QString usn = match.captured(1).trimmed();
        if (usn.contains("uuid:")) {
            int start = usn.indexOf("uuid:") + 5;
            int end = usn.indexOf("::", start);
            if (end == -1) end = usn.length();
            device.name = QString("UPnP Camera (%1)").arg(senderIP);
        }
    }
    
    if (device.name.isEmpty()) {
        device.name = QString("UPnP Camera (%1)").arg(senderIP);
    }
    
    qDebug() << "SSDP: Found camera at" << senderIP << "Location:" << device.serviceUrl;
    emit deviceDiscovered(device);
}

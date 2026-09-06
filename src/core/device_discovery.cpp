#include "device_discovery.h"
#include "local_interfaces.h"
#include <QNetworkDatagram>
#include <QDebug>
#include <QRegularExpression>
#include <QUrl>

DeviceDiscovery::DeviceDiscovery(QObject* parent)
    : QObject(parent)
{
    m_discoveryTimer = new QTimer(this);
    m_discoveryTimer->setSingleShot(true);
    connect(m_discoveryTimer, &QTimer::timeout, this, &DeviceDiscovery::onDiscoveryTimeout);

    // 첫 질의를 놓치는 장치가 있으므로 검색 중에는 주기적으로 다시 보낸다.
    m_resendTimer = new QTimer(this);
    m_resendTimer->setInterval(900);
    connect(m_resendTimer, &QTimer::timeout, this, &DeviceDiscovery::sendQueries);
}

DeviceDiscovery::~DeviceDiscovery() {
    stopDiscovery();
}

void DeviceDiscovery::startDiscovery(int timeout, const QList<QHostAddress>& localAddresses) {
    stopDiscovery();

    m_discovering = true;
    m_discoveredIPs.clear();

    const QList<LocalInterface> interfaces = resolveDiscoveryInterfaces(localAddresses);

    for (const LocalInterface& li : interfaces) {
        auto* ssdp = new QUdpSocket(this);
        if (bindDiscoverySocket(*ssdp, li)) {
            connect(ssdp, &QUdpSocket::readyRead, this, &DeviceDiscovery::onSsdpReadyRead);
            m_ssdpSockets.append(ssdp);
        } else {
            delete ssdp;
        }

        // 5353 이 아닌 포트에서 보내는 "legacy unicast" 질의. 응답자는 유니캐스트로 답해야 한다(RFC 6762 6.7).
        auto* mdns = new QUdpSocket(this);
        if (bindDiscoverySocket(*mdns, li)) {
            connect(mdns, &QUdpSocket::readyRead, this, &DeviceDiscovery::onMdnsReadyRead);
            m_mdnsSockets.append(mdns);
        } else {
            delete mdns;
        }

        emit info(QString("mDNS/SSDP on %1").arg(li.label()));
    }

    // 멀티캐스트로만 답하는 장치를 위해 5353 그룹도 듣는다. 다른 mDNS 서비스가 포트를 독점하면 실패해도 진행한다.
    m_mdnsListener = new QUdpSocket(this);
    if (m_mdnsListener->bind(QHostAddress::AnyIPv4, MDNS_PORT,
                             QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        for (const LocalInterface& li : interfaces) {
            m_mdnsListener->joinMulticastGroup(QHostAddress(MDNS_ADDRESS), li.iface);
        }
        connect(m_mdnsListener, &QUdpSocket::readyRead, this, &DeviceDiscovery::onMdnsReadyRead);
    } else {
        qWarning() << "mDNS: listener bind on 5353 failed:" << m_mdnsListener->errorString();
        delete m_mdnsListener;
        m_mdnsListener = nullptr;
    }

    if (m_ssdpSockets.isEmpty() && m_mdnsSockets.isEmpty()) {
        m_discovering = false;
        emit error("No usable IPv4 network interface for discovery");
        emit discoveryFinished();
        return;
    }

    qInfo() << "DeviceDiscovery: SSDP sockets" << m_ssdpSockets.size()
            << "mDNS sockets" << m_mdnsSockets.size() << "timeout" << timeout << "ms";

    sendQueries();
    m_resendTimer->start();
    m_discoveryTimer->start(timeout);
}

void DeviceDiscovery::stopDiscovery() {
    m_discoveryTimer->stop();
    m_resendTimer->stop();
    m_discovering = false;

    for (QUdpSocket* socket : m_ssdpSockets) {
        socket->close();
        socket->deleteLater();
    }
    m_ssdpSockets.clear();

    for (QUdpSocket* socket : m_mdnsSockets) {
        socket->close();
        socket->deleteLater();
    }
    m_mdnsSockets.clear();

    if (m_mdnsListener) {
        m_mdnsListener->close();
        m_mdnsListener->deleteLater();
        m_mdnsListener = nullptr;
    }
}

QByteArray DeviceDiscovery::buildMdnsQuery() {
    QByteArray query;

    query.append('\x00'); query.append('\x00');   // Transaction ID
    query.append('\x00'); query.append('\x00');   // Flags: standard query
    query.append('\x00'); query.append('\x03');   // Questions: 3
    query.append('\x00'); query.append('\x00');   // Answer RRs
    query.append('\x00'); query.append('\x00');   // Authority RRs
    query.append('\x00'); query.append('\x00');   // Additional RRs

    auto appendQuestion = [&query](const QStringList& labels) {
        for (const QString& label : labels) {
            QByteArray data = label.toUtf8();
            query.append(static_cast<char>(data.length()));
            query.append(data);
        }
        query.append('\x00');                          // End of name
        query.append('\x00'); query.append('\x0c');    // Type: PTR
        // Class: IN + QU(unicast-response) 비트. 응답이 이 소켓으로 유니캐스트된다.
        query.append(static_cast<char>(0x80)); query.append('\x01');
    };

    appendQuestion({"_rtsp", "_tcp", "local"});
    appendQuestion({"_axis-video", "_tcp", "local"});
    appendQuestion({"_http", "_tcp", "local"});

    return query;
}

void DeviceDiscovery::sendQueries() {
    static const QByteArray kSsdpRequests[] = {
        QByteArray("M-SEARCH * HTTP/1.1\r\n"
                   "HOST: 239.255.255.250:1900\r\n"
                   "MAN: \"ssdp:discover\"\r\n"
                   "MX: 3\r\n"
                   "ST: ssdp:all\r\n"
                   "\r\n"),
        QByteArray("M-SEARCH * HTTP/1.1\r\n"
                   "HOST: 239.255.255.250:1900\r\n"
                   "MAN: \"ssdp:discover\"\r\n"
                   "MX: 3\r\n"
                   "ST: urn:schemas-upnp-org:device:MediaServer:1\r\n"
                   "\r\n"),
        QByteArray("M-SEARCH * HTTP/1.1\r\n"
                   "HOST: 239.255.255.250:1900\r\n"
                   "MAN: \"ssdp:discover\"\r\n"
                   "MX: 3\r\n"
                   "ST: urn:schemas-upnp-org:device:Basic:1\r\n"
                   "\r\n"),
    };

    for (QUdpSocket* socket : m_ssdpSockets) {
        for (const QByteArray& request : kSsdpRequests) {
            if (socket->writeDatagram(request, QHostAddress(SSDP_ADDRESS), SSDP_PORT) < 0) {
                qWarning() << "SSDP: send failed from" << socket->localAddress().toString()
                           << socket->errorString();
                break;
            }
        }
    }

    const QByteArray mdnsQuery = buildMdnsQuery();
    for (QUdpSocket* socket : m_mdnsSockets) {
        if (socket->writeDatagram(mdnsQuery, QHostAddress(MDNS_ADDRESS), MDNS_PORT) < 0) {
            qWarning() << "mDNS: send failed from" << socket->localAddress().toString()
                       << socket->errorString();
        }
    }
}

void DeviceDiscovery::onMdnsReadyRead() {
    auto* socket = qobject_cast<QUdpSocket*>(sender());
    if (!socket) {
        return;
    }
    while (socket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = socket->receiveDatagram();
        parseMdnsResponse(datagram.data(), datagram.senderAddress());
    }
}

void DeviceDiscovery::onSsdpReadyRead() {
    auto* socket = qobject_cast<QUdpSocket*>(sender());
    if (!socket) {
        return;
    }
    while (socket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = socket->receiveDatagram();
        parseSsdpResponse(datagram.data(), datagram.senderAddress());
    }
}

void DeviceDiscovery::onDiscoveryTimeout() {
    qDebug() << "DeviceDiscovery: timeout, finishing";
    stopDiscovery();
    emit discoveryFinished();
}

void DeviceDiscovery::parseMdnsResponse(const QByteArray& data, const QHostAddress& sender) {
    if (data.size() < 12) return;

    QString senderIP = sender.toString();
    if (senderIP.startsWith("::ffff:")) {
        senderIP = senderIP.mid(7);
    }

    if (m_discoveredIPs.contains(senderIP)) return;

    // QR 비트가 없으면 (다른 호스트의) 질의이므로 무시
    quint16 flags = (static_cast<quint8>(data[2]) << 8) | static_cast<quint8>(data[3]);
    if (!(flags & 0x8000)) return;

    quint16 answerCount = (static_cast<quint8>(data[6]) << 8) | static_cast<quint8>(data[7]);
    if (answerCount == 0) return;

    QString dataStr = QString::fromUtf8(data);

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
    device.port = 554;
    device.name = QString("mDNS Camera (%1)").arg(senderIP);

    qInfo() << "mDNS: found camera at" << senderIP;
    emit deviceDiscovered(device);
}

void DeviceDiscovery::parseSsdpResponse(const QByteArray& data, const QHostAddress& sender) {
    QString response = QString::fromUtf8(data);

    if (!response.startsWith("HTTP/1.1 200") && !response.startsWith("NOTIFY")) {
        return;
    }

    QString senderIP = sender.toString();
    if (senderIP.startsWith("::ffff:")) {
        senderIP = senderIP.mid(7);
    }

    if (m_discoveredIPs.contains(senderIP)) return;

    bool isExcluded = response.contains("InternetGatewayDevice", Qt::CaseInsensitive) ||
                      response.contains("igd.xml", Qt::CaseInsensitive) ||          // 공유기(IGD) 설명 문서
                      response.contains("vxWorks", Qt::CaseInsensitive) ||          // 저가 공유기 펌웨어
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
    device.port = 554;

    QRegularExpression locationRegex("LOCATION:\\s*(.+)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = locationRegex.match(response);
    if (match.hasMatch()) {
        device.serviceUrl = match.captured(1).trimmed();
        QUrl url(device.serviceUrl);
        if (url.port() > 0) {
            device.port = url.port();
        }
    }

    QRegularExpression serverRegex("SERVER:\\s*(.+)", QRegularExpression::CaseInsensitiveOption);
    match = serverRegex.match(response);
    if (match.hasMatch()) {
        device.model = match.captured(1).trimmed();
    }

    device.name = QString("UPnP Camera (%1)").arg(senderIP);

    qInfo() << "SSDP: found camera at" << senderIP << "location:" << device.serviceUrl;
    emit deviceDiscovered(device);
}

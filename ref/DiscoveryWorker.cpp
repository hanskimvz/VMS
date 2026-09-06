#include "io/DiscoveryWorker.h"

#include "core/DiscoverySettings.h"

#include <functional>

#include <QEventLoop>
#include <QElapsedTimer>
#include <QHostAddress>
#include <QNetworkAccessManager>
#include <QNetworkDatagram>
#include <QNetworkInterface>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QRegularExpression>
#include <QSet>
#include <QTcpSocket>
#include <QTimer>
#include <QUdpSocket>
#include <QUuid>
#include <QUrl>

#include <memory>

namespace {

struct DiscoveryHit
{
    QString ip;
    QString usn;
    QString mac;
    QString model;
    QString brand;
    QString friendlyName;
    QString firmware;
    QString uptime;
    QString rtspUrl;
    QString url;
    QString source;
};

constexpr int kSsdpTimeoutMs = 3500;
constexpr int kOnvifTimeoutMs = 3000;
constexpr int kHttpTimeoutMs = 1000;

QString normalizeMac(QString mac)
{
    mac.remove(QLatin1Char('-'));
    mac.remove(QLatin1Char(':'));
    mac = mac.trimmed().toUpper();

    if (mac.size() != 12) {
        return mac;
    }

    QStringList parts;
    parts.reserve(6);
    for (int i = 0; i < 12; i += 2) {
        parts.append(mac.mid(i, 2));
    }
    return parts.join(QLatin1Char(':'));
}

QString extractIpFromUrl(const QString &urlString)
{
    const QUrl url(urlString.trimmed());
    if (!url.host().isEmpty()) {
        return url.host();
    }

    static const QRegularExpression ipRegex(
        QStringLiteral(R"((\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}))"));
    const QRegularExpressionMatch match = ipRegex.match(urlString);
    if (match.hasMatch()) {
        return match.captured(1);
    }

    return {};
}

QString headerValueFromBlock(const QString &block, const QString &headerName)
{
    const QStringList lines = block.split(QRegularExpression(QStringLiteral("[\r\n]+")),
                                          Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        const int colon = line.indexOf(QLatin1Char(':'));
        if (colon <= 0) {
            continue;
        }

        const QString name = line.left(colon).trimmed();
        if (name.compare(headerName, Qt::CaseInsensitive) == 0) {
            return line.mid(colon + 1).trimmed();
        }
    }
    return {};
}

QString xmlTextForLocalName(const QByteArray &xml, const QString &localName)
{
    static const QRegularExpression elementRegex(
        QStringLiteral(R"(<(?:[\w.-]+:)?%1[^>]*>([\s\S]*?)</(?:[\w.-]+:)?%1>)"));
    const QRegularExpression regex(elementRegex.pattern().arg(QRegularExpression::escape(localName)));
    const QRegularExpressionMatch match = regex.match(QString::fromUtf8(xml));
    if (match.hasMatch()) {
        return match.captured(1).trimmed();
    }
    return {};
}

QString xmlTextForElement(const QByteArray &xml, const QString &elementName)
{
    return xmlTextForLocalName(xml, elementName);
}

bool isUsableDiscoveryIp(const QString &ip)
{
    if (ip.isEmpty()) {
        return false;
    }

    QHostAddress address;
    if (!address.setAddress(ip) || address.protocol() != QAbstractSocket::IPv4Protocol) {
        return false;
    }

    // ONVIF XAddrs often include unreachable link-local addresses (169.254.x.x).
    if (ip.startsWith(QStringLiteral("169.254."))) {
        return false;
    }

    return true;
}

QString onvifScopeValue(const QString &scopes, const QString &key)
{
    const QStringList tokens =
        scopes.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    const QString prefix = QStringLiteral("onvif://www.onvif.org/") + key + QLatin1Char('/');
    for (const QString &token : tokens) {
        if (token.startsWith(prefix, Qt::CaseInsensitive)) {
            return token.mid(prefix.size());
        }
    }
    return {};
}

bool parseMacFromOnvifEndpoint(const QString &endpointAddress, QString *macOut)
{
    static const QRegularExpression endpointMacRegex(
        QStringLiteral(R"(8000-([0-9A-Fa-f]{12}))"));
    const QRegularExpressionMatch match = endpointMacRegex.match(endpointAddress);
    if (!match.hasMatch() || macOut == nullptr) {
        return false;
    }

    *macOut = normalizeMac(match.captured(1));
    return macOut->size() == 17;
}

bool parseMacFromFriendlyName(const QString &friendlyName, QString *macOut)
{
    static const QRegularExpression macRegex(
        QStringLiteral(R"(((?:[0-9A-Fa-f]{2}[-:]){5}[0-9A-Fa-f]{2}))"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch match = macRegex.match(friendlyName);
    if (!match.hasMatch() || macOut == nullptr) {
        return false;
    }

    *macOut = normalizeMac(match.captured(1));
    return true;
}

bool isBareUsn(const QString &value)
{
    static const QRegularExpression usnRegex(QStringLiteral(R"(^[A-Za-z0-9]{6,16}$)"));
    return usnRegex.match(value.trimmed()).hasMatch();
}

QString usnFromMac(const QString &mac)
{
    const QString normalized = normalizeMac(mac);
    if (normalized.size() != 17) {
        return {};
    }

    const QString suffix = normalized.mid(9).remove(QLatin1Char(':'));
    if (suffix.size() != 6) {
        return {};
    }

    return (QStringLiteral("HA0A") + suffix).toUpper();
}

bool isOnvifHardwareId(const QString &value)
{
    if (value.isEmpty()) {
        return false;
    }

    static const QRegularExpression numericRegex(QStringLiteral(R"(^\d+$)"));
    return numericRegex.match(value.trimmed()).hasMatch();
}

bool parseMacFromSigmaStarSerial(const QString &serialNumber, QString *macOut)
{
    if (macOut == nullptr || serialNumber.isEmpty()) {
        return false;
    }

    const QString lastSegment = serialNumber.section(QLatin1Char('-'), -1).trimmed();
    if (lastSegment.size() != 12) {
        return false;
    }

    static const QRegularExpression hexRegex(QStringLiteral(R"(^[0-9A-Fa-f]{12}$)"));
    if (!hexRegex.match(lastSegment).hasMatch()) {
        return false;
    }

    *macOut = normalizeMac(lastSegment);
    return macOut->size() == 17;
}

QString sigmaStarUsnFromSerial(const QString &serialNumber, const QString &friendlyName)
{
    const QString suffix = friendlyName.section(QLatin1Char('-'), -1).trimmed();
    if (!suffix.isEmpty() && suffix != friendlyName && suffix.size() >= 6) {
        return suffix.toUpper();
    }

    const QString lastSegment = serialNumber.section(QLatin1Char('-'), -1).trimmed();
    if (lastSegment.size() == 12) {
        return lastSegment.toUpper();
    }

    return serialNumber;
}

bool parseStarLightUsn(const QString &usnHeader, QString *usnOut, QString *macOut);
bool parseSigmaStarUsn(const QString &usnHeader, QString *usnOut, QString *macOut);

void applyUpnpFields(DiscoveryHit *hit, const QByteArray &body, const QString &urlString)
{
    if (hit == nullptr) {
        return;
    }

    const QString model = xmlTextForElement(body, QStringLiteral("modelName"));
    const QString friendlyName = xmlTextForElement(body, QStringLiteral("friendlyName"));
    const QString manufacturer = xmlTextForElement(body, QStringLiteral("manufacturer"));
    const QString serialNumber = xmlTextForElement(body, QStringLiteral("serialNumber"));
    const QString udn = xmlTextForElement(body, QStringLiteral("UDN"));
    const QString modelNumber = xmlTextForElement(body, QStringLiteral("modelNumber"));
    const QString uptime = xmlTextForElement(body, QStringLiteral("uptime"));
    const QString rtspUrl = xmlTextForElement(body, QStringLiteral("rtspUrl"));
    const QString deviceType = xmlTextForElement(body, QStringLiteral("deviceType"));

    if (model.isEmpty() && serialNumber.isEmpty() && udn.isEmpty()) {
        return;
    }

    if (!deviceType.contains(QStringLiteral("nvcdevice"), Qt::CaseInsensitive)
        && !deviceType.contains(QStringLiteral("Basic"), Qt::CaseInsensitive)
        && model.isEmpty()) {
        return;
    }

    if (!model.isEmpty()) {
        hit->model = model;
    }
    if (!friendlyName.isEmpty()) {
        hit->friendlyName = friendlyName;
    }
    if (!manufacturer.isEmpty()) {
        hit->brand = manufacturer;
    }
    if (!modelNumber.isEmpty()) {
        hit->firmware = modelNumber;
    }
    if (!uptime.isEmpty()) {
        hit->uptime = uptime;
    }
    if (!rtspUrl.isEmpty()) {
        hit->rtspUrl = rtspUrl;
    }

    QString parsedUsn;
    QString parsedMac;
    if (parseStarLightUsn(udn, &parsedUsn, &parsedMac)
        || parseStarLightUsn(serialNumber, &parsedUsn, &parsedMac)
        || parseSigmaStarUsn(udn, &parsedUsn, &parsedMac)
        || parseSigmaStarUsn(serialNumber, &parsedUsn, &parsedMac)) {
        if (hit->usn.isEmpty()) {
            hit->usn = parsedUsn;
        }
        if (hit->mac.isEmpty()) {
            hit->mac = parsedMac;
        }
    } else if (isBareUsn(serialNumber)) {
        if (hit->usn.isEmpty()) {
            hit->usn = serialNumber.toUpper();
        }
    } else if (!serialNumber.isEmpty()) {
        QString sigmaMac;
        if (parseMacFromSigmaStarSerial(serialNumber, &sigmaMac)) {
            if (hit->mac.isEmpty()) {
                hit->mac = sigmaMac;
            }
            if (hit->usn.isEmpty()) {
                hit->usn = sigmaStarUsnFromSerial(serialNumber, friendlyName);
            }
        } else if (hit->usn.isEmpty()) {
            hit->usn = sigmaStarUsnFromSerial(serialNumber, friendlyName);
        }
    }

    if (hit->mac.isEmpty()) {
        parseMacFromFriendlyName(friendlyName, &hit->mac);
    }
    if (hit->usn.isEmpty() && !hit->mac.isEmpty()) {
        hit->usn = usnFromMac(hit->mac);
    }

    hit->url = urlString;
    if (hit->ip.isEmpty()) {
        hit->ip = extractIpFromUrl(urlString);
    }
}

DiscoveryHit parseOnvifProbeMatch(const QByteArray &payload)
{
    DiscoveryHit hit;
    hit.source = QStringLiteral("ONVIF");

    const QString xaddrs = xmlTextForLocalName(payload, QStringLiteral("XAddrs"));
    const QStringList urls =
        xaddrs.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    for (const QString &url : urls) {
        const QString ip = extractIpFromUrl(url);
        if (!isUsableDiscoveryIp(ip)) {
            continue;
        }

        hit.ip = ip;
        if (!url.contains(QStringLiteral("/onvif/"), Qt::CaseInsensitive)) {
            hit.url = url;
        }
        break;
    }

    if (hit.ip.isEmpty()) {
        return hit;
    }

    const QString scopes = xmlTextForLocalName(payload, QStringLiteral("Scopes"));
    const QString hardware = onvifScopeValue(scopes, QStringLiteral("hardware"));
    const QString name = onvifScopeValue(scopes, QStringLiteral("name"));
    // ONVIF hardware scope is often a numeric board ID (e.g. 10001), not the product name.
    // Windows Explorer uses UPnP description.xml modelName — enrich later overwrites this.
    if (!hardware.isEmpty() && !isOnvifHardwareId(hardware)) {
        hit.model = hardware;
    }
    if (!name.isEmpty()) {
        hit.friendlyName = name;
    }

    const QString endpointAddress = xmlTextForLocalName(payload, QStringLiteral("Address"));
    parseMacFromOnvifEndpoint(endpointAddress, &hit.mac);
    if (hit.usn.isEmpty() && !hit.mac.isEmpty()) {
        hit.usn = usnFromMac(hit.mac);
    }

    return hit;
}

bool isValidDeviceHit(const DiscoveryHit &hit)
{
    return isUsableDiscoveryIp(hit.ip)
        && (!hit.model.isEmpty() || !hit.usn.isEmpty() || !hit.mac.isEmpty());
}

DeviceRecord toDeviceRecord(const DiscoveryHit &hit)
{
    DeviceRecord record;
    record.productName = hit.model;
    record.ipAddress = hit.ip;
    record.macAddress = hit.mac;
    record.usn = hit.usn;
    record.friendlyName = hit.friendlyName.isEmpty() ? hit.brand : hit.friendlyName;
    record.firmware = hit.firmware;
    record.uptime = hit.uptime;
    record.rtspUrl = hit.rtspUrl;
    return record;
}

bool isStarLightMac(const QString &mac)
{
    const QString normalized = mac.trimmed().toUpper();
    return normalized.startsWith(QStringLiteral("00:13:2"))
           || normalized.startsWith(QStringLiteral("00:30:1B"));
}

bool ipv4ToUInt(const QString &ip, quint32 *value)
{
    const QStringList parts = ip.split(QLatin1Char('.'));
    if (parts.size() != 4 || value == nullptr) {
        return false;
    }

    quint32 result = 0;
    for (const QString &part : parts) {
        bool ok = false;
        const int octet = part.toInt(&ok);
        if (!ok || octet < 0 || octet > 255) {
            return false;
        }
        result = (result << 8) | static_cast<quint32>(octet);
    }

    *value = result;
    return true;
}

QString ipv4FromUInt(quint32 value)
{
    return QStringLiteral("%1.%2.%3.%4")
        .arg((value >> 24) & 0xFF)
        .arg((value >> 16) & 0xFF)
        .arg((value >> 8) & 0xFF)
        .arg(value & 0xFF);
}

QList<QString> enumerateIpv4Range(const QString &range, int maxHosts = 512)
{
    QList<QString> ips;
    if (range.trimmed().isEmpty()) {
        return ips;
    }

    const int slash = range.indexOf(QLatin1Char('/'));
    if (slash <= 0) {
        return ips;
    }

    quint32 networkValue = 0;
    if (!ipv4ToUInt(range.left(slash), &networkValue)) {
        return ips;
    }

    bool ok = false;
    int prefix = range.mid(slash + 1).toInt(&ok);
    if (!ok || prefix < 8 || prefix > 32) {
        return ips;
    }

    if (prefix < 24) {
        prefix = 24;
    }

    const quint32 mask = prefix == 32 ? 0xFFFFFFFFU : (~0U << (32 - prefix));
    const quint32 network = networkValue & mask;
    const quint32 broadcast = network | ~mask;
    const quint32 firstHost = network + 1;
    const quint32 lastHost = broadcast - 1;
    const quint32 limit = qMin(lastHost, firstHost + static_cast<quint32>(maxHosts) - 1U);

    for (quint32 value = firstHost; value <= lastHost && value <= limit; ++value) {
        ips.append(ipv4FromUInt(value));
    }

    return ips;
}

void logLocalInterfaces(const std::function<void(const QString &)> &logFn)
{
    if (!logFn) {
        return;
    }

    logFn(QStringLiteral("[Discovery] local IPv4 interfaces:"));
    for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {
        if (!(iface.flags() & QNetworkInterface::IsUp)
            || (iface.flags() & QNetworkInterface::IsLoopBack)) {
            continue;
        }

        for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
            if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol) {
                continue;
            }

            logFn(QStringLiteral("[Discovery]   %1 %2/%3")
                      .arg(iface.humanReadableName(), entry.ip().toString())
                      .arg(entry.prefixLength()));
        }
    }
}

bool isTcpPortOpen(const QString &ip, quint16 port, int timeoutMs)
{
    QTcpSocket socket;
    socket.connectToHost(ip, port);
    const bool connected = socket.waitForConnected(timeoutMs);
    socket.abort();
    return connected;
}

bool configureDiscoveryUdpSocket(QUdpSocket &socket,
                                 const SelectedDiscoveryInterface &selected,
                                 const std::function<void(const QString &)> &logFn)
{
    const QHostAddress bindAddress =
        selected.isValid ? selected.localAddress : QHostAddress(QHostAddress::AnyIPv4);

    auto tryBind = [&](const QHostAddress &address) {
        return socket.bind(address, 0, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    };

    if (!tryBind(bindAddress)) {
        if (logFn) {
            logFn(QStringLiteral("[Discovery] UDP bind failed on %1: %2")
                      .arg(bindAddress.toString(), socket.errorString()));
        }
        if (bindAddress != QHostAddress(QHostAddress::AnyIPv4) && tryBind(QHostAddress(QHostAddress::AnyIPv4))) {
            if (logFn) {
                logFn(QStringLiteral("[Discovery] UDP bind fallback: 0.0.0.0"));
            }
        } else {
            return false;
        }
    }

    if (selected.isValid) {
        socket.setMulticastInterface(selected.interface);
        if (logFn) {
            logFn(QStringLiteral("[Discovery] using NIC: %1 (%2)")
                      .arg(selected.interface.humanReadableName(),
                           selected.localAddress.toString()));
        }
    }

    return true;
}

bool isStarLightSsdpSt(const QString &st)
{
    if (st.isEmpty()) {
        return true;
    }

    return st.contains(QStringLiteral("upnp:rootdevice"), Qt::CaseInsensitive)
           || st.contains(QStringLiteral("nvcdevice"), Qt::CaseInsensitive)
           || st.contains(QStringLiteral("device:Basic"), Qt::CaseInsensitive)
           || st.contains(QStringLiteral("nvccontrol"), Qt::CaseInsensitive);
}

bool isStarLightSsdpResponse(const QString &block)
{
    const QString usn = headerValueFromBlock(block, QStringLiteral("USN"));
    const QString st = headerValueFromBlock(block, QStringLiteral("ST"));
    const QString location = headerValueFromBlock(block, QStringLiteral("LOCATION"));
    const QString server = headerValueFromBlock(block, QStringLiteral("SERVER"));

    if (parseStarLightUsn(usn, nullptr, nullptr)) {
        return true;
    }
    if (usn.contains(QStringLiteral("00:13:23"), Qt::CaseInsensitive)
        || usn.contains(QStringLiteral("00-13-23"), Qt::CaseInsensitive)) {
        return true;
    }
    if (usn.contains(QStringLiteral("00301B"), Qt::CaseInsensitive)
        || usn.contains(QStringLiteral("00:30:1B"), Qt::CaseInsensitive)
        || usn.contains(QStringLiteral("00-30-1B"), Qt::CaseInsensitive)) {
        return true;
    }
    if (st.contains(QStringLiteral("nvcdevice"), Qt::CaseInsensitive)) {
        return true;
    }
    if (location.contains(QStringLiteral("upnpdevicedesc.xml"), Qt::CaseInsensitive)
        && (usn.contains(QStringLiteral("HA0A"), Qt::CaseInsensitive)
            || usn.contains(QStringLiteral("G90A"), Qt::CaseInsensitive))) {
        return true;
    }
    if (server.contains(QStringLiteral("IPNX"), Qt::CaseInsensitive)) {
        return true;
    }
    if (server.contains(QStringLiteral("SigmaStar"), Qt::CaseInsensitive)
        && location.contains(QStringLiteral("description.xml"), Qt::CaseInsensitive)) {
        return true;
    }
    return false;
}

bool parseStarLightUsn(const QString &usnHeader, QString *usnOut, QString *macOut)
{
    /* IPNX 00:13:23 and SLC 00:30:1B (any MAC, hyphen or colon, case-insensitive). */
    static const QRegularExpression usnRegex(
        QStringLiteral(R"(uuid:([A-Za-z0-9]+)-((?:[0-9A-Fa-f]{2}[-:]){5}[0-9A-Fa-f]{2}))"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch match = usnRegex.match(usnHeader);
    if (!match.hasMatch()) {
        return false;
    }

    if (usnOut != nullptr) {
        *usnOut = match.captured(1).toUpper();
    }
    if (macOut != nullptr) {
        *macOut = normalizeMac(match.captured(2));
    }
    return true;
}

bool parseSigmaStarUsn(const QString &usnHeader, QString *usnOut, QString *macOut)
{
    static const QRegularExpression usnRegex(
        QStringLiteral(R"(uuid:[0-9A-Fa-f-]*?(00301[Bb][0-9A-Fa-f]{6}))"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch match = usnRegex.match(usnHeader);
    if (!match.hasMatch()) {
        return false;
    }

    const QString hex = match.captured(1).toUpper();
    if (usnOut != nullptr) {
        *usnOut = hex;
    }
    if (macOut != nullptr) {
        *macOut = normalizeMac(hex);
    }
    return true;
}

void applySsdpIdentity(DiscoveryHit *hit, const QString &usnHeader)
{
    if (hit == nullptr) {
        return;
    }

    QString usn;
    QString mac;
    if (parseStarLightUsn(usnHeader, &usn, &mac) || parseSigmaStarUsn(usnHeader, &usn, &mac)) {
        hit->usn = usn;
        hit->mac = mac;
    }
}

class DiscoveryRunner
{
public:
    explicit DiscoveryRunner(const std::function<void(const QString &)> &logFn,
                             QString ipRange = QString())
        : m_logFn(logFn)
        , m_ipRange(std::move(ipRange))
    {
    }

    QList<DeviceRecord> run(const std::function<void(const QString &)> &statusFn)
    {
        m_statusFn = statusFn;
        m_hits.clear();
        m_seenIps.clear();

        logLocalInterfaces(m_logFn);
        log(QStringLiteral("[Discovery] ===== START ====="));
        if (!m_ipRange.isEmpty()) {
            log(QStringLiteral("[Discovery] IP range hint: %1").arg(m_ipRange));
        }

        const DiscoverySettings discoverySettings = DiscoverySettingsStore::load();
        m_discoveryInterfaces = NetworkInterfaceSelector::selectAll(discoverySettings.interfaceName);
        if (m_discoveryInterfaces.isEmpty()) {
            const SelectedDiscoveryInterface fallback =
                NetworkInterfaceSelector::select(m_ipRange, discoverySettings.interfaceName);
            if (fallback.isValid) {
                m_discoveryInterfaces.append(fallback);
            }
        }

        if (m_discoveryInterfaces.isEmpty()) {
            log(QStringLiteral("[Discovery] WARNING: no usable NIC for range %1")
                    .arg(m_ipRange.isEmpty() ? QStringLiteral("(any)") : m_ipRange));
        } else {
            for (const SelectedDiscoveryInterface &selected : m_discoveryInterfaces) {
                log(QStringLiteral("[Discovery] probe NIC: %1 (%2)")
                        .arg(selected.interface.humanReadableName(),
                             selected.localAddress.toString()));
            }
        }

        setStatus(QStringLiteral("SSDP"));
        log(QStringLiteral("[Discovery] --- SSDP M-SEARCH ---"));
        mergeHits(discoverBySsdp(kSsdpTimeoutMs), QStringLiteral("SSDP"));

        setStatus(QStringLiteral("ONVIF"));
        log(QStringLiteral("[Discovery] --- ONVIF WS-Discovery ---"));
        mergeHits(discoverByOnvif(kOnvifTimeoutMs), QStringLiteral("ONVIF"));

        setStatus(QStringLiteral("ARP"));
        log(QStringLiteral("[Discovery] --- ARP supplement ---"));
        mergeHits(discoverByArp(), QStringLiteral("ARP"));

        if (m_hits.isEmpty() && !m_ipRange.isEmpty()) {
            setStatus(QStringLiteral("HTTP"));
            log(QStringLiteral("[Discovery] --- IP range HTTP probe (multicast fallback) ---"));
            mergeHits(probeIpRange(m_ipRange), QStringLiteral("HTTP"));
        }

        setStatus(QStringLiteral("UPnP"));
        log(QStringLiteral("[Discovery] --- UPnP description enrich ---"));
        enrichHitsWithUpnp();

        QList<DeviceRecord> devices;
        for (const DiscoveryHit &hit : m_hits) {
            if (!isValidDeviceHit(hit)) {
                log(QStringLiteral("[Discovery] skip invalid device ip=%1").arg(hit.ip));
                continue;
            }

            devices.append(toDeviceRecord(hit));
            log(QStringLiteral("[Discovery] DEVICE ip=%1 usn=%2 mac=%3 model=%4 fw=%5 source=%6")
                    .arg(hit.ip, hit.usn, hit.mac, hit.model, hit.firmware, hit.source));
        }

        log(QStringLiteral("[Discovery] ===== DONE (%1 device(s)) =====").arg(devices.size()));
        return devices;
    }

private:
    void log(const QString &line) const
    {
        if (m_logFn) {
            m_logFn(line);
        }
    }

    void setStatus(const QString &status) const
    {
        if (m_statusFn) {
            m_statusFn(status);
        }
    }

    void mergeHits(const QList<DiscoveryHit> &found, const QString &sourceTag)
    {
        for (DiscoveryHit hit : found) {
            if (!isUsableDiscoveryIp(hit.ip)) {
                continue;
            }

            bool merged = false;
            for (DiscoveryHit &existing : m_hits) {
                if (existing.ip != hit.ip) {
                    continue;
                }

                if (existing.model.isEmpty() || isOnvifHardwareId(existing.model)) {
                    if (!hit.model.isEmpty() && !isOnvifHardwareId(hit.model)) {
                        existing.model = hit.model;
                    }
                }
                if (existing.usn.isEmpty()) {
                    existing.usn = hit.usn;
                }
                if (existing.mac.isEmpty()) {
                    existing.mac = hit.mac;
                }
                if (existing.friendlyName.isEmpty()) {
                    existing.friendlyName = hit.friendlyName;
                }
                if (existing.firmware.isEmpty()) {
                    existing.firmware = hit.firmware;
                }
                if (existing.uptime.isEmpty()) {
                    existing.uptime = hit.uptime;
                }
                if (existing.rtspUrl.isEmpty()) {
                    existing.rtspUrl = hit.rtspUrl;
                }
                if (existing.url.isEmpty() && !hit.url.contains(QStringLiteral("/onvif/"))) {
                    existing.url = hit.url;
                }
                merged = true;
                break;
            }

            if (merged || m_seenIps.contains(hit.ip)) {
                continue;
            }

            if (hit.source.isEmpty()) {
                hit.source = sourceTag;
            }

            m_seenIps.insert(hit.ip);
            m_hits.append(hit);
        }
    }

    QList<DiscoveryHit> discoverBySsdp(int timeoutMs)
    {
        QList<DiscoveryHit> hits;
        std::vector<std::unique_ptr<QUdpSocket>> sockets = bindDiscoverySockets();
        if (sockets.empty()) {
            return hits;
        }

        const QByteArray request = QByteArray(
            "M-SEARCH * HTTP/1.1\r\n"
            "HOST: 239.255.255.250:1900\r\n"
            "MAN: \"ssdp:discover\"\r\n"
            "MX: 3\r\n"
            "ST: ssdp:all\r\n"
            "\r\n");

        auto sendSearch = [&]() {
            bool sent = false;
            for (const auto &socket : sockets) {
                log(QStringLiteral("[SSDP] >> multicast 239.255.255.250:1900 from %1")
                        .arg(socket->localAddress().toString()));
                if (socket->writeDatagram(request, QHostAddress(QStringLiteral("239.255.255.250")),
                                          1900)
                    < 0) {
                    log(QStringLiteral("[SSDP] send failed: %1").arg(socket->errorString()));
                    continue;
                }
                sent = true;
            }
            return sent;
        };

        log(QString::fromUtf8(request));
        if (!sendSearch()) {
            return hits;
        }

        QElapsedTimer elapsed;
        elapsed.start();
        QElapsedTimer resendTimer;
        resendTimer.start();

        int rawDatagrams = 0;
        while (elapsed.elapsed() < timeoutMs) {
            if (resendTimer.elapsed() >= 900) {
                sendSearch();
                resendTimer.restart();
            }

            bool hadDatagram = false;
            for (const auto &socket : sockets) {
                while (socket->hasPendingDatagrams()) {
                    hadDatagram = true;
                    ++rawDatagrams;
                    const QNetworkDatagram datagram = socket->receiveDatagram();
                    const QByteArray payload = datagram.data();
                    const QString block = QString::fromUtf8(payload);

                    log(QStringLiteral("[SSDP] << from %1:%2")
                            .arg(datagram.senderAddress().toString())
                            .arg(datagram.senderPort()));
                    log(block);

                    if (!isStarLightSsdpResponse(block)) {
                        log(QStringLiteral("[SSDP] skip non-StarLight device"));
                        continue;
                    }

                    const QString st = headerValueFromBlock(block, QStringLiteral("ST"));
                    if (!isStarLightSsdpSt(st)) {
                        log(QStringLiteral("[SSDP] skip ST=%1").arg(st));
                        continue;
                    }

                    const QString location = headerValueFromBlock(block, QStringLiteral("LOCATION"));
                    const QString usnHeader = headerValueFromBlock(block, QStringLiteral("USN"));
                    const QString ip = extractIpFromUrl(location);
                    if (!isUsableDiscoveryIp(ip)) {
                        continue;
                    }

                    DiscoveryHit hit;
                    hit.ip = ip;
                    hit.url = location;
                    hit.source = QStringLiteral("SSDP");
                    applySsdpIdentity(&hit, usnHeader);
                    hits.append(hit);
                }
            }

            if (!hadDatagram) {
                for (const auto &socket : sockets) {
                    if (socket->waitForReadyRead(40)) {
                        break;
                    }
                }
            }
        }

        log(QStringLiteral("[SSDP] raw datagrams=%1, accepted=%2")
                .arg(rawDatagrams)
                .arg(hits.size()));
        if (rawDatagrams == 0) {
            log(QStringLiteral("[SSDP] no UDP responses — check firewall/VPN/same subnet"));
        }
        return hits;
    }

    QList<DiscoveryHit> discoverByOnvif(int timeoutMs)
    {
        QList<DiscoveryHit> hits;
        std::vector<std::unique_ptr<QUdpSocket>> sockets = bindDiscoverySockets();
        if (sockets.empty()) {
            return hits;
        }

        const QString messageId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const QByteArray request = QString(
                                      R"(<?xml version="1.0" encoding="UTF-8"?>)"
                                      R"(<e:Envelope xmlns:e="http://www.w3.org/2003/05/soap-envelope")"
                                      R"( xmlns:w="http://schemas.xmlsoap.org/ws/2004/08/addressing")"
                                      R"( xmlns:d="http://schemas.xmlsoap.org/ws/2005/04/discovery")"
                                      R"( xmlns:dn="http://www.onvif.org/ver10/network/wsdl">)"
                                      R"(<e:Header>)"
                                      R"(<w:MessageID>uuid:%1</w:MessageID>)"
                                      R"(<w:To e:mustUnderstand="true">urn:schemas-xmlsoap-org:ws:2005:04:discovery</w:To>)"
                                      R"(<w:Action e:mustUnderstand="true">http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe</w:Action>)"
                                      R"(</e:Header>)"
                                      R"(<e:Body>)"
                                      R"(<d:Probe>)"
                                      R"(<d:Types>dn:NetworkVideoTransmitter</d:Types>)"
                                      R"(</d:Probe>)"
                                      R"(</e:Body>)"
                                      R"(</e:Envelope>)")
                                      .arg(messageId)
                                      .toUtf8();

        log(QStringLiteral("[ONVIF] >> multicast 239.255.255.250:3702"));
        log(QString::fromUtf8(request));

        bool sent = false;
        for (const auto &socket : sockets) {
            if (socket->writeDatagram(request, QHostAddress(QStringLiteral("239.255.255.250")), 3702)
                < 0) {
                log(QStringLiteral("[ONVIF] send failed on %1: %2")
                        .arg(socket->localAddress().toString(), socket->errorString()));
                continue;
            }
            sent = true;
        }
        if (!sent) {
            return hits;
        }

        QElapsedTimer elapsed;
        elapsed.start();

        while (elapsed.elapsed() < timeoutMs) {
            bool hadDatagram = false;
            for (const auto &socket : sockets) {
                while (socket->hasPendingDatagrams()) {
                    hadDatagram = true;
                    const QNetworkDatagram datagram = socket->receiveDatagram();
                    const QByteArray payload = datagram.data();

                    log(QStringLiteral("[ONVIF] << from %1:%2")
                            .arg(datagram.senderAddress().toString())
                            .arg(datagram.senderPort()));
                    log(QString::fromUtf8(payload));

                    DiscoveryHit hit = parseOnvifProbeMatch(payload);
                    if (!isValidDeviceHit(hit)) {
                        continue;
                    }

                    hits.append(hit);
                }
            }

            if (!hadDatagram) {
                for (const auto &socket : sockets) {
                    if (socket->waitForReadyRead(40)) {
                        break;
                    }
                }
            }
        }

        log(QStringLiteral("[ONVIF] found %1 device(s)").arg(hits.size()));
        return hits;
    }

    QList<DiscoveryHit> discoverByArp()
    {
        QList<DiscoveryHit> hits;

        QProcess process;
#ifdef Q_OS_WIN
        process.start(QStringLiteral("cmd"), {QStringLiteral("/c"),
                                              QStringLiteral("arp -a | findstr /i \"00-13-2 00-30-1\"")});
#else
        process.start(QStringLiteral("sh"),
                      {QStringLiteral("-c"),
                       QStringLiteral("arp -n | grep -Ei '00:13:2|00:30:1b'")});
#endif
        process.waitForFinished(5000);

        const QString output = QString::fromLocal8Bit(process.readAllStandardOutput());
        log(QStringLiteral("[ARP] command output:"));
        log(output);

        static const QRegularExpression lineRegex(
            QStringLiteral(R"(([0-9.]+)\s+([\w:-]+))"), QRegularExpression::CaseInsensitiveOption);

        QSet<QString> localSeen;
        const QStringList lines = output.split(QRegularExpression(QStringLiteral("[\r\n]+")),
                                               Qt::SkipEmptyParts);
        for (QString line : lines) {
            line.replace(QStringLiteral("ether"), QString());
            line.replace(QLatin1Char('-'), QString());
            line.replace(QLatin1Char(':'), QString());

            const QRegularExpressionMatch match = lineRegex.match(line);
            if (!match.hasMatch()) {
                continue;
            }

            const QString ip = match.captured(1);
            const QString mac = normalizeMac(match.captured(2));
            if (!isStarLightMac(mac)) {
                continue;
            }
            if (localSeen.contains(ip)) {
                continue;
            }

            log(QStringLiteral("[ARP] candidate ip=%1 mac=%2").arg(ip, mac));

            localSeen.insert(ip);

            DiscoveryHit hit;
            hit.ip = ip;
            hit.mac = mac;
            hit.source = QStringLiteral("ARP");
            tryFetchUpnpDescription(ip, &hit);
            hits.append(hit);
        }

        log(QStringLiteral("[ARP] found %1 device(s)").arg(hits.size()));
        return hits;
    }

    QList<DiscoveryHit> probeIpRange(const QString &range)
    {
        QList<DiscoveryHit> hits;
        const QList<QString> ips = enumerateIpv4Range(range);
        log(QStringLiteral("[HTTP] probing %1 address(es) in %2").arg(ips.size()).arg(range));

        for (const QString &ip : ips) {
            if (!isTcpPortOpen(ip, 49152, 200) && !isTcpPortOpen(ip, 80, 200)) {
                continue;
            }

            log(QStringLiteral("[HTTP] reachable ip=%1 — UPnP probe").arg(ip));

            DiscoveryHit hit;
            hit.ip = ip;
            hit.source = QStringLiteral("HTTP");
            if (!tryFetchUpnpDescription(ip, &hit) || !isValidDeviceHit(hit)) {
                log(QStringLiteral("[HTTP] skip ip=%1 (no StarLight UPnP desc)").arg(ip));
                continue;
            }

            hits.append(hit);
            log(QStringLiteral("[HTTP] found ip=%1 model=%2").arg(hit.ip, hit.model));
        }

        log(QStringLiteral("[HTTP] found %1 device(s)").arg(hits.size()));
        return hits;
    }

    void enrichHitsWithUpnp()
    {
        for (DiscoveryHit &hit : m_hits) {
            tryFetchUpnpDescription(hit.ip, &hit);
        }
    }

    bool tryFetchUpnpDescription(const QString &ip, DiscoveryHit *hit)
    {
        if (hit == nullptr) {
            return false;
        }

        if (!hit->url.isEmpty() && !hit->url.contains(QStringLiteral("/onvif/"), Qt::CaseInsensitive)) {
            if (fetchUpnpFromUrl(hit->url, hit)) {
                return true;
            }
        }

        const QStringList ports = {QStringLiteral("49152"), QStringLiteral("80"),
                                   QStringLiteral("49153")};
        const QStringList pages = {QStringLiteral("upnpdevicedesc.xml"),
                                   QStringLiteral("description.xml"),
                                   QStringLiteral("DigitalSecurityCamera1.xml")};

        for (const QString &port : ports) {
            for (const QString &page : pages) {
                const QString url = QStringLiteral("http://%1:%2/%3").arg(ip, port, page);
                if (fetchUpnpFromUrl(url, hit)) {
                    return true;
                }
            }
        }

        return false;
    }

    bool fetchUpnpFromUrl(const QString &urlString, DiscoveryHit *hit)
    {
        QNetworkAccessManager nam;
        const QUrl url(urlString);
        QNetworkRequest request{url};
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        request.setTransferTimeout(kHttpTimeoutMs);
#endif

        log(QStringLiteral("[UPnP] GET %1").arg(urlString));

        QNetworkReply *reply = nam.get(request);

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        QTimer timeoutTimer;
        timeoutTimer.setSingleShot(true);
        timeoutTimer.start(kHttpTimeoutMs);
        connect(&timeoutTimer, &QTimer::timeout, reply, &QNetworkReply::abort);
#endif

        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        const QByteArray body = reply->readAll();
        const int statusCode =
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        log(QStringLiteral("[UPnP] status=%1 bytes=%2").arg(statusCode).arg(body.size()));
        log(QString::fromUtf8(body));

        const bool ok = reply->error() == QNetworkReply::NoError && !body.isEmpty();
        const QString errorText = reply->errorString();
        reply->deleteLater();

        if (!ok) {
            log(QStringLiteral("[UPnP] request failed: %1").arg(errorText));
            return false;
        }

        applyUpnpFields(hit, body, urlString);
        return !hit->model.isEmpty() || !hit->usn.isEmpty();
    }

    std::vector<std::unique_ptr<QUdpSocket>> bindDiscoverySockets()
    {
        std::vector<std::unique_ptr<QUdpSocket>> sockets;
        QList<SelectedDiscoveryInterface> interfaces = m_discoveryInterfaces;
        if (interfaces.isEmpty()) {
            interfaces.append(SelectedDiscoveryInterface());
        }

        for (const SelectedDiscoveryInterface &selected : interfaces) {
            auto socket = std::make_unique<QUdpSocket>();
            if (!configureDiscoveryUdpSocket(*socket, selected, m_logFn)) {
                continue;
            }

            log(QStringLiteral("[Discovery] bound %1 port %2")
                    .arg(selected.isValid ? selected.localAddress.toString()
                                          : QStringLiteral("0.0.0.0"))
                    .arg(socket->localPort()));
            sockets.push_back(std::move(socket));
        }

        return sockets;
    }

    std::function<void(const QString &)> m_logFn;
    std::function<void(const QString &)> m_statusFn;
    QString m_ipRange;
    QList<SelectedDiscoveryInterface> m_discoveryInterfaces;
    QList<DiscoveryHit> m_hits;
    QSet<QString> m_seenIps;
};

} // namespace

DiscoveryWorker::DiscoveryWorker(QObject *parent)
    : QObject(parent)
{
}

void DiscoveryWorker::runDiscovery(const QString &ipRange)
{
    DiscoveryRunner runner([this](const QString &line) { emit logLine(line); }, ipRange);
    const QList<DeviceRecord> devices = runner.run(
        [this](const QString &status) { emit statusChanged(status); });
    emit finished(devices);
}

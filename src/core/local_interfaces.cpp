#include "local_interfaces.h"
#include <QDebug>
#include <QVariant>

QString LocalInterface::label() const {
    return QString("%1 (%2)").arg(address.toString(), iface.humanReadableName());
}

QList<LocalInterface> discoveryInterfaces() {
    QList<LocalInterface> result;

    const QList<QNetworkInterface> all = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface& iface : all) {
        const QNetworkInterface::InterfaceFlags flags = iface.flags();
        if (!(flags & QNetworkInterface::IsUp) ||
            !(flags & QNetworkInterface::IsRunning) ||
            (flags & QNetworkInterface::IsLoopBack)) {
            continue;
        }

        const QList<QNetworkAddressEntry> entries = iface.addressEntries();
        for (const QNetworkAddressEntry& entry : entries) {
            const QHostAddress ip = entry.ip();
            if (ip.protocol() != QAbstractSocket::IPv4Protocol) {
                continue;
            }
            if (ip.isLoopback() || ip.isLinkLocal() || ip.isNull()) {
                continue;
            }

            LocalInterface li;
            li.iface = iface;
            li.address = ip;
            li.prefixLength = entry.prefixLength();
            result.append(li);
        }
    }

    return result;
}

QList<LocalInterface> resolveDiscoveryInterfaces(const QList<QHostAddress>& addresses) {
    const QList<LocalInterface> all = discoveryInterfaces();
    if (addresses.isEmpty()) {
        return all;
    }

    QList<LocalInterface> result;
    for (const LocalInterface& li : all) {
        if (addresses.contains(li.address)) {
            result.append(li);
        }
    }
    return result;
}

bool bindDiscoverySocket(QUdpSocket& socket, const LocalInterface& li, quint16 port) {
    if (!socket.bind(li.address, port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        qWarning() << "Discovery: bind failed on" << li.label() << socket.errorString();
        return false;
    }

    // 바인드 주소만으로는 멀티캐스트 송신 인터페이스가 정해지지 않는다(IP_MULTICAST_IF 는 별도).
    socket.setMulticastInterface(li.iface);
    socket.setSocketOption(QAbstractSocket::MulticastTtlOption, QVariant(4));
    return true;
}

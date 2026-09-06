#ifndef LOCAL_INTERFACES_H
#define LOCAL_INTERFACES_H

#include <QHostAddress>
#include <QNetworkInterface>
#include <QUdpSocket>
#include <QList>
#include <QString>

// 장치 검색(멀티캐스트)에 쓸 수 있는 로컬 IPv4 인터페이스 하나.
// 한 어댑터에 IPv4 주소가 여럿이면 주소마다 항목이 생긴다.
struct LocalInterface {
    QNetworkInterface iface;
    QHostAddress address;
    int prefixLength = 0;

    QString label() const;   // "192.168.1.3 (Ethernet)"
};

// Up + Running, 루프백 아님, IPv4, 링크로컬(169.254.x.x) 아님.
QList<LocalInterface> discoveryInterfaces();

// 주소 목록에 해당하는 인터페이스만 돌려준다. 목록이 비어 있으면 전체.
QList<LocalInterface> resolveDiscoveryInterfaces(const QList<QHostAddress>& addresses);

// 소켓을 인터페이스 주소에 바인드하고 멀티캐스트 송신 인터페이스로 지정한다.
// 0.0.0.0 에 바인드한 소켓은 OS 기본 경로(VPN, 가상 어댑터일 수 있음)로만 멀티캐스트를 내보내므로
// 카메라가 있는 LAN 으로 보내려면 이 함수를 써야 한다.
bool bindDiscoverySocket(QUdpSocket& socket, const LocalInterface& li, quint16 port = 0);

#endif // LOCAL_INTERFACES_H

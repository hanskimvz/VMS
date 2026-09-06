#ifndef STREAM_HEALTH_CHECKER_H
#define STREAM_HEALTH_CHECKER_H

#include <QObject>
#include <QSet>
#include <QString>
#include <QThreadPool>
#include <atomic>

// RTSP 스트림을 실제로 열어 보고(DESCRIBE/SETUP/PLAY + 첫 패킷) 받을 수 있는지 확인한다.
// 라이브 뷰와 무관하게 메인/서브 스트림 각각의 도달 여부를 알기 위한 것이다.
// 프로브는 스레드 풀에서 돌고 결과는 이 객체가 속한 스레드에서 streamChecked 로 나온다.
class StreamHealthChecker : public QObject {
    Q_OBJECT

public:
    explicit StreamHealthChecker(QObject* parent = nullptr);
    ~StreamHealthChecker();

    // 같은 (cameraId, subStream) 프로브가 진행 중이면 중복 요청은 무시된다.
    void check(const QString& cameraId, bool subStream,
               const QString& url, const QString& username, const QString& password);
    bool isPending(const QString& cameraId, bool subStream) const;

    // 프로브 한 건의 상한(접속 + 스트림 정보 읽기). 소켓 타임아웃(5초)보다 조금 길게 잡는다.
    static constexpr int PROBE_TIMEOUT_MS = 8000;

signals:
    void streamChecked(const QString& cameraId, bool subStream, bool ok, const QString& error);

private:
    class ProbeTask;
    friend class ProbeTask;

    static QString pendingKey(const QString& cameraId, bool subStream);
    void onProbeFinished(const QString& cameraId, bool subStream, bool ok, const QString& error);

    QThreadPool m_pool;
    std::atomic<bool> m_abort{false};
    QSet<QString> m_pending;   // 이 객체의 스레드에서만 접근
};

#endif // STREAM_HEALTH_CHECKER_H

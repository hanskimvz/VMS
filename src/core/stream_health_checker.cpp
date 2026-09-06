#include "stream_health_checker.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QRunnable>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
}

namespace {

struct ProbeContext {
    QElapsedTimer timer;
    std::atomic<bool>* abort = nullptr;
};

int probeInterrupt(void* opaque) {
    auto* ctx = static_cast<ProbeContext*>(opaque);
    if (ctx->abort && ctx->abort->load()) {
        return 1;
    }
    return ctx->timer.elapsed() > StreamHealthChecker::PROBE_TIMEOUT_MS ? 1 : 0;
}

QString avErrorString(int ret) {
    char buf[256];
    av_strerror(ret, buf, sizeof(buf));
    return QString::fromUtf8(buf);
}

// StreamReceiver::open() 과 같은 순서로 열어 본다. 디코더는 만들지 않는다.
// 실패 사유에 URL 이나 비밀번호가 섞이지 않도록 av_strerror 문자열만 돌려준다.
bool probeStream(const QString& url, const QString& username, const QString& password,
                 std::atomic<bool>* abort, QString& error) {
    QString fullUrl = url;
    if (!username.isEmpty() && !url.contains("@")) {
        int protoEnd = url.indexOf("://");
        if (protoEnd > 0) {
            fullUrl = url.left(protoEnd + 3) + username + ":" + password + "@" + url.mid(protoEnd + 3);
        }
    }

    ProbeContext ctx;
    ctx.timer.start();
    ctx.abort = abort;

    AVFormatContext* fmt = avformat_alloc_context();
    if (!fmt) {
        error = "Failed to allocate format context";
        return false;
    }
    fmt->interrupt_callback.callback = &probeInterrupt;
    fmt->interrupt_callback.opaque = &ctx;

    AVDictionary* options = nullptr;
    av_dict_set(&options, "rtsp_transport", "tcp", 0);
    av_dict_set(&options, "timeout", "5000000", 0);

    int ret = avformat_open_input(&fmt, fullUrl.toUtf8().constData(), nullptr, &options);
    av_dict_free(&options);
    if (ret < 0) {
        error = avErrorString(ret);
        return false;   // 실패 시 avformat_open_input 이 fmt 를 해제한다.
    }

    // 접속은 됐지만 패킷이 오지 않는 경우(인코더 꺼짐 등)는 여기서 걸러진다.
    ret = avformat_find_stream_info(fmt, nullptr);
    if (ret < 0) {
        error = QString("No stream info: %1").arg(avErrorString(ret));
        avformat_close_input(&fmt);
        return false;
    }

    bool hasVideo = false;
    for (unsigned int i = 0; i < fmt->nb_streams; i++) {
        const AVCodecParameters* par = fmt->streams[i]->codecpar;
        if (par->codec_type == AVMEDIA_TYPE_VIDEO && par->width > 0 && par->height > 0) {
            hasVideo = true;
            break;
        }
    }
    avformat_close_input(&fmt);

    if (!hasVideo) {
        error = "No decodable video stream";
        return false;
    }
    return true;
}

} // namespace

class StreamHealthChecker::ProbeTask : public QRunnable {
public:
    ProbeTask(StreamHealthChecker* owner, QString cameraId, bool subStream,
              QString url, QString username, QString password)
        : m_owner(owner), m_cameraId(std::move(cameraId)), m_subStream(subStream)
        , m_url(std::move(url)), m_username(std::move(username)), m_password(std::move(password))
    {
        setAutoDelete(true);
    }

    void run() override {
        QString error;
        bool ok = probeStream(m_url, m_username, m_password, &m_owner->m_abort, error);
        if (m_owner->m_abort.load()) {
            return;   // 종료 중이면 결과를 보내지 않는다.
        }
        // owner 가 먼저 파괴되면 큐에 남은 호출은 Qt 가 버린다.
        QMetaObject::invokeMethod(m_owner, [owner = m_owner, id = m_cameraId, sub = m_subStream, ok, error]() {
            owner->onProbeFinished(id, sub, ok, error);
        }, Qt::QueuedConnection);
    }

private:
    StreamHealthChecker* m_owner;
    QString m_cameraId;
    bool m_subStream;
    QString m_url;
    QString m_username;
    QString m_password;
};

StreamHealthChecker::StreamHealthChecker(QObject* parent)
    : QObject(parent)
{
    // 16채널 × 2스트림을 한꺼번에 열지 않도록 동시 프로브 수를 제한한다.
    m_pool.setMaxThreadCount(4);
}

StreamHealthChecker::~StreamHealthChecker() {
    m_abort = true;          // 진행 중인 프로브는 interrupt callback 으로 즉시 빠져나온다.
    m_pool.clear();          // 아직 시작하지 않은 작업은 버린다.
    m_pool.waitForDone();
}

QString StreamHealthChecker::pendingKey(const QString& cameraId, bool subStream) {
    return cameraId + (subStream ? "/sub" : "/main");
}

bool StreamHealthChecker::isPending(const QString& cameraId, bool subStream) const {
    return m_pending.contains(pendingKey(cameraId, subStream));
}

void StreamHealthChecker::check(const QString& cameraId, bool subStream,
                                const QString& url, const QString& username, const QString& password) {
    const QString key = pendingKey(cameraId, subStream);
    if (m_pending.contains(key)) {
        return;
    }
    m_pending.insert(key);
    m_pool.start(new ProbeTask(this, cameraId, subStream, url, username, password));
}

void StreamHealthChecker::onProbeFinished(const QString& cameraId, bool subStream, bool ok, const QString& error) {
    m_pending.remove(pendingKey(cameraId, subStream));
    emit streamChecked(cameraId, subStream, ok, error);
}

#ifndef STREAM_RECEIVER_H
#define STREAM_RECEIVER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QImage>
#include <QElapsedTimer>
#include <atomic>
#include <memory>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

struct VideoFrame {
    QImage image;
    int64_t pts = 0;
    int64_t dts = 0;
    bool isKeyFrame = false;
};

struct StreamStats {
    int64_t framesReceived = 0;
    int64_t framesDecoded = 0;
    int64_t framesDropped = 0;
    int64_t decodeErrors = 0;
    int64_t networkErrors = 0;
    double currentFps = 0;
    double avgDecodeTimeMs = 0;
    int64_t totalBytesReceived = 0;
};

class StreamReceiver : public QThread {
    Q_OBJECT

public:
    explicit StreamReceiver(QObject* parent = nullptr);
    ~StreamReceiver();

    bool open(const QString& url, const QString& username = "", const QString& password = "");
    void close();

    void start();
    void stop();

    // 수신 루프가 돌고 있는지. 스레드 자체의 실행 여부는 QThread::isRunning() 을 쓴다.
    bool isStreaming() const { return m_running; }
    bool isOpened() const { return m_formatCtx != nullptr; }

    int width() const { return m_width; }
    int height() const { return m_height; }
    double fps() const { return m_fps; }

    StreamStats getStats() const;
    void resetStats();

signals:
    void frameReady(const VideoFrame& frame);
    void error(const QString& message);
    void connected();
    void disconnected();
    void statsUpdated(const StreamStats& stats);

protected:
    void run() override;

private:
    static int interruptCallback(void* opaque);

    bool initDecoder();
    bool initScaler(int width, int height, AVPixelFormat format);
    void cleanup();
    int decodePacket(AVPacket* packet);
    QImage convertToQImage(AVFrame* frame);
    void updateStats();

    QString m_url;
    QString m_username;
    QString m_password;

    AVFormatContext* m_formatCtx = nullptr;
    AVCodecContext* m_codecCtx = nullptr;
    SwsContext* m_swsCtx = nullptr;
    AVFrame* m_frame = nullptr;
    AVFrame* m_frameRGB = nullptr;
    uint8_t* m_buffer = nullptr;

    int m_videoStreamIndex = -1;
    int m_width = 0;
    int m_height = 0;
    double m_fps = 0;

    // 현재 스케일러가 받아들이는 입력 형식. 스트림 중간에 해상도/픽셀 포맷이 바뀌면 재생성한다.
    int m_swsWidth = 0;
    int m_swsHeight = 0;
    AVPixelFormat m_swsSrcFormat = AV_PIX_FMT_NONE;

    std::atomic<bool> m_running{false};
    std::atomic<bool> m_abortRequested{false};
    QMutex m_mutex;

    mutable QMutex m_statsMutex;
    StreamStats m_stats;
    QElapsedTimer m_fpsTimer;
    int64_t m_fpsFrameCount = 0;
    double m_totalDecodeTime = 0;
};

#endif // STREAM_RECEIVER_H

#ifndef STREAM_RECEIVER_H
#define STREAM_RECEIVER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QImage>
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

class StreamReceiver : public QThread {
    Q_OBJECT
    
public:
    explicit StreamReceiver(QObject* parent = nullptr);
    ~StreamReceiver();
    
    bool open(const QString& url, const QString& username = "", const QString& password = "");
    void close();
    
    void start();
    void stop();
    
    bool isRunning() const { return m_running; }
    bool isOpened() const { return m_formatCtx != nullptr; }
    
    int width() const { return m_width; }
    int height() const { return m_height; }
    double fps() const { return m_fps; }
    
signals:
    void frameReady(const VideoFrame& frame);
    void error(const QString& message);
    void connected();
    void disconnected();
    
protected:
    void run() override;
    
private:
    bool initDecoder();
    void cleanup();
    bool decodeFrame(AVPacket* packet, VideoFrame& outFrame);
    QImage convertToQImage(AVFrame* frame);
    
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
    
    std::atomic<bool> m_running{false};
    QMutex m_mutex;
};

#endif // STREAM_RECEIVER_H

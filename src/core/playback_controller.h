#ifndef PLAYBACK_CONTROLLER_H
#define PLAYBACK_CONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QImage>
#include <memory>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

class PlaybackController : public QObject {
    Q_OBJECT
    
public:
    enum State {
        Stopped,
        Playing,
        Paused
    };
    
    explicit PlaybackController(QObject* parent = nullptr);
    ~PlaybackController();
    
    bool openFile(const QString& filePath);
    void close();
    
    void play();
    void pause();
    void stop();
    void seek(int64_t timestamp);
    void setSpeed(double speed);
    
    State state() const { return m_state; }
    double speed() const { return m_speed; }
    int64_t duration() const { return m_duration; }
    int64_t currentPosition() const { return m_currentPos; }
    
signals:
    void frameReady(const QImage& frame);
    void positionChanged(int64_t position);
    void stateChanged(State state);
    void error(const QString& message);
    void endOfFile();
    
private slots:
    void onTimer();
    
private:
    bool decodeNextFrame();
    void cleanup();
    
    AVFormatContext* m_formatCtx = nullptr;
    AVCodecContext* m_codecCtx = nullptr;
    SwsContext* m_swsCtx = nullptr;
    AVFrame* m_frame = nullptr;
    AVFrame* m_frameRGB = nullptr;
    uint8_t* m_buffer = nullptr;
    
    int m_videoStreamIndex = -1;
    int m_width = 0;
    int m_height = 0;
    
    State m_state = Stopped;
    double m_speed = 1.0;
    int64_t m_duration = 0;
    int64_t m_currentPos = 0;
    
    QTimer* m_timer;
    double m_frameInterval = 40.0;
};

#endif // PLAYBACK_CONTROLLER_H

#ifndef VIDEO_DECODER_H
#define VIDEO_DECODER_H

#include <QObject>
#include <QImage>
#include <QMutex>
#include <memory>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

class VideoDecoder : public QObject {
    Q_OBJECT
    
public:
    explicit VideoDecoder(QObject* parent = nullptr);
    ~VideoDecoder();
    
    bool initialize(AVCodecID codecId, int width, int height);
    bool decode(const uint8_t* data, int size, QImage& outImage);
    void flush();
    void reset();
    
    int width() const { return m_width; }
    int height() const { return m_height; }
    
private:
    void cleanup();
    
    AVCodecContext* m_codecCtx = nullptr;
    SwsContext* m_swsCtx = nullptr;
    AVFrame* m_frame = nullptr;
    AVFrame* m_frameRGB = nullptr;
    uint8_t* m_buffer = nullptr;
    
    int m_width = 0;
    int m_height = 0;
    
    QMutex m_mutex;
};

#endif // VIDEO_DECODER_H

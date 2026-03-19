#include "video_decoder.h"
#include <QDebug>

VideoDecoder::VideoDecoder(QObject* parent)
    : QObject(parent)
{
}

VideoDecoder::~VideoDecoder() {
    cleanup();
}

bool VideoDecoder::initialize(AVCodecID codecId, int width, int height) {
    QMutexLocker locker(&m_mutex);
    
    cleanup();
    
    const AVCodec* codec = avcodec_find_decoder(codecId);
    if (!codec) {
        qWarning() << "Codec not found:" << codecId;
        return false;
    }
    
    m_codecCtx = avcodec_alloc_context3(codec);
    if (!m_codecCtx) {
        qWarning() << "Failed to allocate codec context";
        return false;
    }
    
    m_codecCtx->width = width;
    m_codecCtx->height = height;
    m_codecCtx->thread_count = 2;
    
    if (avcodec_open2(m_codecCtx, codec, nullptr) < 0) {
        qWarning() << "Failed to open codec";
        cleanup();
        return false;
    }
    
    m_width = width;
    m_height = height;
    
    m_frame = av_frame_alloc();
    m_frameRGB = av_frame_alloc();
    
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB32, width, height, 1);
    m_buffer = (uint8_t*)av_malloc(numBytes);
    
    av_image_fill_arrays(m_frameRGB->data, m_frameRGB->linesize,
                         m_buffer, AV_PIX_FMT_RGB32, width, height, 1);
    
    return true;
}

bool VideoDecoder::decode(const uint8_t* data, int size, QImage& outImage) {
    QMutexLocker locker(&m_mutex);
    
    if (!m_codecCtx) {
        return false;
    }
    
    AVPacket* packet = av_packet_alloc();
    packet->data = const_cast<uint8_t*>(data);
    packet->size = size;
    
    int ret = avcodec_send_packet(m_codecCtx, packet);
    av_packet_free(&packet);
    
    if (ret < 0) {
        return false;
    }
    
    ret = avcodec_receive_frame(m_codecCtx, m_frame);
    if (ret < 0) {
        return false;
    }
    
    if (!m_swsCtx) {
        m_swsCtx = sws_getContext(
            m_frame->width, m_frame->height, (AVPixelFormat)m_frame->format,
            m_width, m_height, AV_PIX_FMT_RGB32,
            SWS_BILINEAR, nullptr, nullptr, nullptr
        );
    }
    
    sws_scale(m_swsCtx,
              m_frame->data, m_frame->linesize, 0, m_frame->height,
              m_frameRGB->data, m_frameRGB->linesize);
    
    outImage = QImage(m_frameRGB->data[0], m_width, m_height,
                      m_frameRGB->linesize[0], QImage::Format_RGB32).copy();
    
    return true;
}

void VideoDecoder::flush() {
    QMutexLocker locker(&m_mutex);
    if (m_codecCtx) {
        avcodec_flush_buffers(m_codecCtx);
    }
}

void VideoDecoder::reset() {
    QMutexLocker locker(&m_mutex);
    cleanup();
}

void VideoDecoder::cleanup() {
    if (m_swsCtx) {
        sws_freeContext(m_swsCtx);
        m_swsCtx = nullptr;
    }
    
    if (m_buffer) {
        av_free(m_buffer);
        m_buffer = nullptr;
    }
    
    if (m_frameRGB) {
        av_frame_free(&m_frameRGB);
    }
    
    if (m_frame) {
        av_frame_free(&m_frame);
    }
    
    if (m_codecCtx) {
        avcodec_free_context(&m_codecCtx);
    }
    
    m_width = 0;
    m_height = 0;
}

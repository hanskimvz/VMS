#include "playback_controller.h"
#include <QDebug>

extern "C" {
#include <libavutil/imgutils.h>
}

PlaybackController::PlaybackController(QObject* parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    connect(m_timer, &QTimer::timeout, this, &PlaybackController::onTimer);
}

PlaybackController::~PlaybackController() {
    close();
}

bool PlaybackController::openFile(const QString& filePath) {
    close();
    
    m_formatCtx = avformat_alloc_context();
    
    int ret = avformat_open_input(&m_formatCtx, filePath.toUtf8().constData(), nullptr, nullptr);
    if (ret < 0) {
        emit error("Failed to open file");
        return false;
    }
    
    ret = avformat_find_stream_info(m_formatCtx, nullptr);
    if (ret < 0) {
        emit error("Failed to find stream info");
        cleanup();
        return false;
    }
    
    for (unsigned int i = 0; i < m_formatCtx->nb_streams; i++) {
        if (m_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            m_videoStreamIndex = i;
            break;
        }
    }
    
    if (m_videoStreamIndex == -1) {
        emit error("No video stream found");
        cleanup();
        return false;
    }
    
    AVCodecParameters* codecpar = m_formatCtx->streams[m_videoStreamIndex]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
    
    if (!codec) {
        emit error("Unsupported codec");
        cleanup();
        return false;
    }
    
    m_codecCtx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(m_codecCtx, codecpar);
    
    if (avcodec_open2(m_codecCtx, codec, nullptr) < 0) {
        emit error("Failed to open codec");
        cleanup();
        return false;
    }
    
    m_width = m_codecCtx->width;
    m_height = m_codecCtx->height;
    
    AVStream* stream = m_formatCtx->streams[m_videoStreamIndex];
    // MKV/TS 는 스트림 단위 duration 이 AV_NOPTS_VALUE 인 경우가 많다. 컨테이너 값으로 폴백한다.
    if (stream->duration != AV_NOPTS_VALUE && stream->duration > 0) {
        m_duration = static_cast<int64_t>(stream->duration * av_q2d(stream->time_base) * 1000);
    } else if (m_formatCtx->duration != AV_NOPTS_VALUE && m_formatCtx->duration > 0) {
        m_duration = m_formatCtx->duration * 1000 / AV_TIME_BASE;
    } else {
        m_duration = 0;
    }
    
    if (stream->avg_frame_rate.den > 0) {
        m_frameInterval = 1000.0 / av_q2d(stream->avg_frame_rate);
    }
    
    m_frame = av_frame_alloc();
    m_frameRGB = av_frame_alloc();
    
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB32, m_width, m_height, 1);
    m_buffer = (uint8_t*)av_malloc(numBytes);
    
    av_image_fill_arrays(m_frameRGB->data, m_frameRGB->linesize,
                         m_buffer, AV_PIX_FMT_RGB32, m_width, m_height, 1);
    
    m_swsCtx = sws_getContext(
        m_width, m_height, m_codecCtx->pix_fmt,
        m_width, m_height, AV_PIX_FMT_RGB32,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );
    
    m_state = Stopped;
    m_currentPos = 0;
    
    return true;
}

void PlaybackController::close() {
    stop();
    cleanup();
}

void PlaybackController::play() {
    if (!m_formatCtx) return;
    
    m_state = Playing;
    m_timer->start(static_cast<int>(m_frameInterval / m_speed));
    emit stateChanged(m_state);
}

void PlaybackController::pause() {
    if (!m_formatCtx) return;
    
    m_state = Paused;
    m_timer->stop();
    emit stateChanged(m_state);
}

void PlaybackController::stop() {
    m_timer->stop();
    m_state = Stopped;
    m_currentPos = 0;
    
    if (m_formatCtx) {
        av_seek_frame(m_formatCtx, m_videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
        avcodec_flush_buffers(m_codecCtx);
    }
    
    emit stateChanged(m_state);
}

void PlaybackController::seek(int64_t timestamp) {
    if (!m_formatCtx) return;
    
    AVStream* stream = m_formatCtx->streams[m_videoStreamIndex];
    int64_t seekTarget = av_rescale_q(timestamp, AV_TIME_BASE_Q, stream->time_base);
    
    if (av_seek_frame(m_formatCtx, m_videoStreamIndex, seekTarget, AVSEEK_FLAG_BACKWARD) >= 0) {
        avcodec_flush_buffers(m_codecCtx);
        m_currentPos = timestamp;
        emit positionChanged(m_currentPos);
        
        if (m_state == Playing) {
            decodeNextFrame();
        }
    }
}

void PlaybackController::setSpeed(double speed) {
    m_speed = qBound(0.25, speed, 4.0);
    
    if (m_timer->isActive()) {
        m_timer->setInterval(static_cast<int>(m_frameInterval / m_speed));
    }
}

void PlaybackController::onTimer() {
    if (!decodeNextFrame()) {
        stop();
        emit endOfFile();
    }
}

bool PlaybackController::decodeNextFrame() {
    if (!m_formatCtx) return false;
    
    AVPacket* packet = av_packet_alloc();
    bool frameDecoded = false;
    
    while (!frameDecoded) {
        int ret = av_read_frame(m_formatCtx, packet);
        
        if (ret < 0) {
            av_packet_free(&packet);
            return false;
        }
        
        if (packet->stream_index == m_videoStreamIndex) {
            ret = avcodec_send_packet(m_codecCtx, packet);
            
            if (ret >= 0) {
                ret = avcodec_receive_frame(m_codecCtx, m_frame);
                
                if (ret >= 0) {
                    sws_scale(m_swsCtx,
                              m_frame->data, m_frame->linesize, 0, m_height,
                              m_frameRGB->data, m_frameRGB->linesize);
                    
                    QImage img(m_frameRGB->data[0], m_width, m_height,
                               m_frameRGB->linesize[0], QImage::Format_RGB32);
                    
                    AVStream* stream = m_formatCtx->streams[m_videoStreamIndex];
                    int64_t ts = m_frame->best_effort_timestamp;
                    if (ts == AV_NOPTS_VALUE) {
                        ts = m_frame->pts;
                    }
                    if (ts != AV_NOPTS_VALUE) {
                        m_currentPos = static_cast<int64_t>(ts * av_q2d(stream->time_base) * 1000);
                    }
                    
                    emit frameReady(img.copy());
                    emit positionChanged(m_currentPos);
                    
                    frameDecoded = true;
                }
            }
        }
        
        av_packet_unref(packet);
    }
    
    av_packet_free(&packet);
    return true;
}

void PlaybackController::cleanup() {
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
    
    if (m_formatCtx) {
        avformat_close_input(&m_formatCtx);
    }
    
    m_videoStreamIndex = -1;
}

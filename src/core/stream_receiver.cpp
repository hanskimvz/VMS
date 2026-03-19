#include "stream_receiver.h"
#include <QDebug>

StreamReceiver::StreamReceiver(QObject* parent)
    : QThread(parent)
{
}

StreamReceiver::~StreamReceiver() {
    stop();
    close();
}

bool StreamReceiver::open(const QString& url, const QString& username, const QString& password) {
    QMutexLocker locker(&m_mutex);
    
    m_url = url;
    m_username = username;
    m_password = password;
    
    QString fullUrl = url;
    if (!username.isEmpty() && !url.contains("@")) {
        int protoEnd = url.indexOf("://");
        if (protoEnd > 0) {
            QString proto = url.left(protoEnd + 3);
            QString rest = url.mid(protoEnd + 3);
            fullUrl = proto + username + ":" + password + "@" + rest;
        }
    }
    
    m_formatCtx = avformat_alloc_context();
    if (!m_formatCtx) {
        emit error("Failed to allocate format context");
        return false;
    }
    
    AVDictionary* options = nullptr;
    av_dict_set(&options, "rtsp_transport", "tcp", 0);
    av_dict_set(&options, "stimeout", "5000000", 0);
    av_dict_set(&options, "max_delay", "500000", 0);
    
    int ret = avformat_open_input(&m_formatCtx, fullUrl.toUtf8().constData(), nullptr, &options);
    av_dict_free(&options);
    
    if (ret < 0) {
        char errbuf[256];
        av_strerror(ret, errbuf, sizeof(errbuf));
        emit error(QString("Failed to open stream: %1").arg(errbuf));
        cleanup();
        return false;
    }
    
    ret = avformat_find_stream_info(m_formatCtx, nullptr);
    if (ret < 0) {
        emit error("Failed to find stream info");
        cleanup();
        return false;
    }
    
    if (!initDecoder()) {
        cleanup();
        return false;
    }
    
    emit connected();
    return true;
}

bool StreamReceiver::initDecoder() {
    m_videoStreamIndex = -1;
    
    for (unsigned int i = 0; i < m_formatCtx->nb_streams; i++) {
        if (m_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            m_videoStreamIndex = i;
            break;
        }
    }
    
    if (m_videoStreamIndex == -1) {
        emit error("No video stream found");
        return false;
    }
    
    AVCodecParameters* codecpar = m_formatCtx->streams[m_videoStreamIndex]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
    
    if (!codec) {
        emit error("Unsupported codec");
        return false;
    }
    
    m_codecCtx = avcodec_alloc_context3(codec);
    if (!m_codecCtx) {
        emit error("Failed to allocate codec context");
        return false;
    }
    
    if (avcodec_parameters_to_context(m_codecCtx, codecpar) < 0) {
        emit error("Failed to copy codec parameters");
        return false;
    }
    
    m_codecCtx->thread_count = 2;
    
    if (avcodec_open2(m_codecCtx, codec, nullptr) < 0) {
        emit error("Failed to open codec");
        return false;
    }
    
    m_width = m_codecCtx->width;
    m_height = m_codecCtx->height;
    
    AVStream* stream = m_formatCtx->streams[m_videoStreamIndex];
    if (stream->avg_frame_rate.den > 0) {
        m_fps = av_q2d(stream->avg_frame_rate);
    } else if (stream->r_frame_rate.den > 0) {
        m_fps = av_q2d(stream->r_frame_rate);
    } else {
        m_fps = 25.0;
    }
    
    m_frame = av_frame_alloc();
    m_frameRGB = av_frame_alloc();
    
    if (!m_frame || !m_frameRGB) {
        emit error("Failed to allocate frames");
        return false;
    }
    
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB32, m_width, m_height, 1);
    m_buffer = (uint8_t*)av_malloc(numBytes);
    
    av_image_fill_arrays(m_frameRGB->data, m_frameRGB->linesize,
                         m_buffer, AV_PIX_FMT_RGB32, m_width, m_height, 1);
    
    m_swsCtx = sws_getContext(
        m_width, m_height, m_codecCtx->pix_fmt,
        m_width, m_height, AV_PIX_FMT_RGB32,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );
    
    if (!m_swsCtx) {
        emit error("Failed to create scaling context");
        return false;
    }
    
    return true;
}

void StreamReceiver::close() {
    QMutexLocker locker(&m_mutex);
    cleanup();
}

void StreamReceiver::cleanup() {
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
    m_width = 0;
    m_height = 0;
}

void StreamReceiver::start() {
    if (!m_running) {
        m_running = true;
        QThread::start();
    }
}

void StreamReceiver::stop() {
    m_running = false;
    if (isRunning()) {
        wait(3000);
        if (isRunning()) {
            terminate();
            wait();
        }
    }
}

void StreamReceiver::run() {
    AVPacket* packet = av_packet_alloc();
    
    while (m_running) {
        QMutexLocker locker(&m_mutex);
        
        if (!m_formatCtx) {
            break;
        }
        
        int ret = av_read_frame(m_formatCtx, packet);
        if (ret < 0) {
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN)) {
                locker.unlock();
                QThread::msleep(10);
                continue;
            }
            emit error("Read frame error");
            break;
        }
        
        if (packet->stream_index == m_videoStreamIndex) {
            VideoFrame frame;
            if (decodeFrame(packet, frame)) {
                locker.unlock();
                emit frameReady(frame);
            }
        }
        
        av_packet_unref(packet);
    }
    
    av_packet_free(&packet);
    emit disconnected();
}

bool StreamReceiver::decodeFrame(AVPacket* packet, VideoFrame& outFrame) {
    int ret = avcodec_send_packet(m_codecCtx, packet);
    if (ret < 0) {
        return false;
    }
    
    ret = avcodec_receive_frame(m_codecCtx, m_frame);
    if (ret < 0) {
        return false;
    }
    
    outFrame.image = convertToQImage(m_frame);
    outFrame.pts = m_frame->pts;
    outFrame.dts = m_frame->pkt_dts;
    outFrame.isKeyFrame = (m_frame->flags & AV_FRAME_FLAG_KEY) != 0;
    
    return !outFrame.image.isNull();
}

QImage StreamReceiver::convertToQImage(AVFrame* frame) {
    sws_scale(m_swsCtx,
              frame->data, frame->linesize, 0, m_height,
              m_frameRGB->data, m_frameRGB->linesize);
    
    QImage img(m_frameRGB->data[0], m_width, m_height, 
               m_frameRGB->linesize[0], QImage::Format_RGB32);
    
    return img.copy();
}

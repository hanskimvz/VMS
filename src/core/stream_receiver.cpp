#include "stream_receiver.h"
#include <QDebug>

StreamReceiver::StreamReceiver(QObject* parent)
    : QThread(parent)
{
    m_fpsTimer.start();
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
    av_dict_set(&options, "max_delay", "100000", 0);
    av_dict_set(&options, "buffer_size", "1024000", 0);
    av_dict_set(&options, "fflags", "nobuffer", 0);
    av_dict_set(&options, "flags", "low_delay", 0);
    av_dict_set(&options, "framedrop", "1", 0);
    
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
    
    m_codecCtx->thread_count = 4;
    m_codecCtx->thread_type = FF_THREAD_FRAME | FF_THREAD_SLICE;
    m_codecCtx->flags |= AV_CODEC_FLAG_LOW_DELAY;
    m_codecCtx->flags2 |= AV_CODEC_FLAG2_FAST;
    
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
    QElapsedTimer decodeTimer;
    int statsCounter = 0;
    
    resetStats();
    m_fpsTimer.restart();
    
    while (m_running) {
        int ret;
        {
            QMutexLocker locker(&m_mutex);
            
            if (!m_formatCtx) {
                break;
            }
            
            ret = av_read_frame(m_formatCtx, packet);
        }
        
        if (ret < 0) {
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN)) {
                QThread::msleep(1);
                continue;
            }
            {
                QMutexLocker statsLock(&m_statsMutex);
                m_stats.networkErrors++;
            }
            qWarning() << "Network read error:" << ret;
            emit error("Read frame error");
            break;
        }
        
        if (packet->stream_index == m_videoStreamIndex) {
            {
                QMutexLocker statsLock(&m_statsMutex);
                m_stats.framesReceived++;
                m_stats.totalBytesReceived += packet->size;
            }
            
            decodeTimer.start();
            
            VideoFrame frame;
            bool decoded = false;
            {
                QMutexLocker locker(&m_mutex);
                decoded = decodeFrame(packet, frame);
            }
            
            if (decoded) {
                double decodeTime = decodeTimer.elapsed();
                
                {
                    QMutexLocker statsLock(&m_statsMutex);
                    m_stats.framesDecoded++;
                    m_fpsFrameCount++;
                    m_totalDecodeTime += decodeTime;
                    m_stats.avgDecodeTimeMs = m_totalDecodeTime / m_stats.framesDecoded;
                }
                
                emit frameReady(frame);
            } else {
                QMutexLocker statsLock(&m_statsMutex);
                m_stats.decodeErrors++;
            }
            
            statsCounter++;
            if (statsCounter >= 30) {
                updateStats();
                statsCounter = 0;
            }
        }
        
        av_packet_unref(packet);
    }
    
    av_packet_free(&packet);
    emit disconnected();
}

StreamStats StreamReceiver::getStats() const {
    QMutexLocker locker(&m_statsMutex);
    return m_stats;
}

void StreamReceiver::resetStats() {
    QMutexLocker locker(&m_statsMutex);
    m_stats = StreamStats();
    m_fpsFrameCount = 0;
    m_totalDecodeTime = 0;
    m_fpsTimer.restart();
}

void StreamReceiver::updateStats() {
    QMutexLocker locker(&m_statsMutex);
    
    qint64 elapsed = m_fpsTimer.elapsed();
    if (elapsed > 0) {
        m_stats.currentFps = (m_fpsFrameCount * 1000.0) / elapsed;
    }
    
    m_fpsFrameCount = 0;
    m_fpsTimer.restart();
    
    m_stats.framesDropped = m_stats.framesReceived - m_stats.framesDecoded - m_stats.decodeErrors;
    if (m_stats.framesDropped < 0) m_stats.framesDropped = 0;
    
    StreamStats statsCopy = m_stats;
    locker.unlock();
    
    emit statsUpdated(statsCopy);
    
    if (m_stats.framesReceived > 0) {
        double dropRate = (m_stats.decodeErrors * 100.0) / m_stats.framesReceived;
        if (dropRate > 5.0) {
            qWarning() << "High decode error rate:" << dropRate << "% - Received:" 
                       << m_stats.framesReceived << "Decoded:" << m_stats.framesDecoded
                       << "Errors:" << m_stats.decodeErrors
                       << "FPS:" << m_stats.currentFps
                       << "Avg decode time:" << m_stats.avgDecodeTimeMs << "ms";
        }
    }
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

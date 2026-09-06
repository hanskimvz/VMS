#include "stream_receiver.h"
#include <QDebug>

extern "C" {
#include <libavutil/dict.h>
}

StreamReceiver::StreamReceiver(QObject* parent)
    : QThread(parent)
{
    m_fpsTimer.start();
}

StreamReceiver::~StreamReceiver() {
    stop();
    close();
}

// FFmpeg 이 블로킹 I/O 중에 주기적으로 호출한다. 1을 돌려주면 진행 중인 호출이
// AVERROR_EXIT 로 즉시 빠져나온다. 덕분에 stop() 이 스레드를 강제 종료할 필요가 없다.
int StreamReceiver::interruptCallback(void* opaque) {
    auto* self = static_cast<StreamReceiver*>(opaque);
    return (self && self->m_abortRequested.load()) ? 1 : 0;
}

bool StreamReceiver::open(const QString& url, const QString& username, const QString& password) {
    QMutexLocker locker(&m_mutex);

    cleanup();
    m_abortRequested = false;

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

    m_formatCtx->interrupt_callback.callback = &StreamReceiver::interruptCallback;
    m_formatCtx->interrupt_callback.opaque = this;

    AVDictionary* options = nullptr;
    av_dict_set(&options, "rtsp_transport", "tcp", 0);
    // 소켓 I/O 타임아웃(µs). FFmpeg 5 부터 "stimeout" 이 아니라 "timeout" 이다.
    av_dict_set(&options, "timeout", "5000000", 0);
    av_dict_set(&options, "max_delay", "100000", 0);
    av_dict_set(&options, "buffer_size", "1024000", 0);
    av_dict_set(&options, "fflags", "nobuffer", 0);
    // (low_delay 는 코덱 옵션이라 여기서는 무시된다. initDecoder() 에서 AV_CODEC_FLAG_LOW_DELAY 로 직접 건다.)

    int ret = avformat_open_input(&m_formatCtx, fullUrl.toUtf8().constData(), nullptr, &options);

    // 열기에 성공했는데도 디먹서가 소비하지 않은 옵션은 이름이 틀린 것이다. 조용히 무시되지 않도록 남긴다.
    // (열기에 실패하면 옵션이 전혀 소비되지 않으므로 검사 의미가 없다.)
    if (ret >= 0) {
        const AVDictionaryEntry* entry = nullptr;
        while ((entry = av_dict_iterate(options, entry)) != nullptr) {
            qWarning() << "StreamReceiver: option not recognized by demuxer:" << entry->key;
        }
    }
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
            m_videoStreamIndex = static_cast<int>(i);
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

    // 채널당 디코더 스레드. 16채널 × 4 = 64개는 과하므로 서브스트림급 해상도는 2개로 제한한다.
    int pixels = codecpar->width * codecpar->height;
    m_codecCtx->thread_count = (pixels > 1280 * 720) ? 4 : 2;
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
    if (stream->avg_frame_rate.den > 0 && stream->avg_frame_rate.num > 0) {
        m_fps = av_q2d(stream->avg_frame_rate);
    } else if (stream->r_frame_rate.den > 0 && stream->r_frame_rate.num > 0) {
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

    // 일부 스트림은 첫 프레임을 디코드하기 전까지 해상도/픽셀 포맷을 모른다.
    // 그 경우 스케일러는 convertToQImage() 에서 첫 프레임을 보고 만든다.
    if (m_width > 0 && m_height > 0 && m_codecCtx->pix_fmt != AV_PIX_FMT_NONE) {
        if (!initScaler(m_width, m_height, m_codecCtx->pix_fmt)) {
            return false;
        }
    }

    return true;
}

bool StreamReceiver::initScaler(int width, int height, AVPixelFormat format) {
    if (m_swsCtx) {
        sws_freeContext(m_swsCtx);
        m_swsCtx = nullptr;
    }
    if (m_buffer) {
        av_free(m_buffer);
        m_buffer = nullptr;
    }
    m_swsWidth = 0;
    m_swsHeight = 0;
    m_swsSrcFormat = AV_PIX_FMT_NONE;

    if (width <= 0 || height <= 0 || format == AV_PIX_FMT_NONE || !m_frameRGB) {
        emit error("Invalid frame format");
        return false;
    }

    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB32, width, height, 1);
    if (numBytes <= 0) {
        emit error("Invalid frame size");
        return false;
    }

    m_buffer = static_cast<uint8_t*>(av_malloc(static_cast<size_t>(numBytes)));
    if (!m_buffer) {
        emit error("Failed to allocate RGB buffer");
        return false;
    }

    av_image_fill_arrays(m_frameRGB->data, m_frameRGB->linesize,
                         m_buffer, AV_PIX_FMT_RGB32, width, height, 1);

    m_swsCtx = sws_getContext(
        width, height, format,
        width, height, AV_PIX_FMT_RGB32,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );

    if (!m_swsCtx) {
        emit error("Failed to create scaling context");
        return false;
    }

    m_swsWidth = width;
    m_swsHeight = height;
    m_swsSrcFormat = format;
    m_width = width;
    m_height = height;
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
    m_swsWidth = 0;
    m_swsHeight = 0;
    m_swsSrcFormat = AV_PIX_FMT_NONE;
}

void StreamReceiver::start() {
    if (QThread::isRunning()) {
        return;
    }
    m_abortRequested = false;
    m_running = true;
    QThread::start();
}

void StreamReceiver::stop() {
    m_running = false;
    m_abortRequested = true;

    if (QThread::isRunning()) {
        // interrupt callback 덕분에 av_read_frame 은 곧 AVERROR_EXIT 로 돌아온다.
        // terminate() 는 뮤텍스를 잠근 채 스레드를 죽여 close() 를 영원히 막으므로 쓰지 않는다.
        if (!wait(5000)) {
            qWarning() << "StreamReceiver: worker did not stop within 5s, waiting further";
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
            if (ret == AVERROR(EAGAIN)) {
                QThread::msleep(1);
                continue;
            }
            if (ret == AVERROR_EXIT || m_abortRequested) {
                break;  // stop() 요청
            }

            {
                QMutexLocker statsLock(&m_statsMutex);
                m_stats.networkErrors++;
            }

            if (ret == AVERROR_EOF) {
                qWarning() << "StreamReceiver: stream ended:" << m_url;
                emit error("Stream ended");
            } else {
                char errbuf[256];
                av_strerror(ret, errbuf, sizeof(errbuf));
                qWarning() << "StreamReceiver: read error:" << errbuf << m_url;
                emit error(QString("Read frame error: %1").arg(errbuf));
            }
            break;
        }

        if (packet->stream_index == m_videoStreamIndex) {
            {
                QMutexLocker statsLock(&m_statsMutex);
                m_stats.framesReceived++;
                m_stats.totalBytesReceived += packet->size;
            }

            decodeTimer.start();

            int decodedFrames;
            {
                QMutexLocker locker(&m_mutex);
                decodedFrames = decodePacket(packet);
            }

            if (decodedFrames < 0) {
                QMutexLocker statsLock(&m_statsMutex);
                m_stats.decodeErrors++;
            } else if (decodedFrames > 0) {
                double decodeTime = static_cast<double>(decodeTimer.elapsed());
                QMutexLocker statsLock(&m_statsMutex);
                m_totalDecodeTime += decodeTime;
                if (m_stats.framesDecoded > 0) {
                    m_stats.avgDecodeTimeMs = m_totalDecodeTime / static_cast<double>(m_stats.framesDecoded);
                }
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
    m_running = false;
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
        m_stats.currentFps = (static_cast<double>(m_fpsFrameCount) * 1000.0) / static_cast<double>(elapsed);
    }

    m_fpsFrameCount = 0;
    m_fpsTimer.restart();

    m_stats.framesDropped = m_stats.framesReceived - m_stats.framesDecoded - m_stats.decodeErrors;
    if (m_stats.framesDropped < 0) m_stats.framesDropped = 0;

    StreamStats statsCopy = m_stats;
    locker.unlock();

    emit statsUpdated(statsCopy);

    if (statsCopy.framesReceived > 0) {
        double errorRate = (static_cast<double>(statsCopy.decodeErrors) * 100.0) / static_cast<double>(statsCopy.framesReceived);
        if (errorRate > 5.0) {
            qWarning() << "High decode error rate:" << errorRate << "% - Received:"
                       << statsCopy.framesReceived << "Decoded:" << statsCopy.framesDecoded
                       << "Errors:" << statsCopy.decodeErrors
                       << "FPS:" << statsCopy.currentFps
                       << "Avg decode time:" << statsCopy.avgDecodeTimeMs << "ms";
        }
    }
}

// 패킷 하나를 디코더에 넣고, 나오는 프레임을 전부 꺼내 emit 한다.
// 프레임 스레딩이 켜져 있으면 초기 몇 패킷은 프레임 없이 EAGAIN 만 돌려주는데, 이는 오류가 아니다.
// 반환값: emit 한 프레임 수, 디코더 오류면 -1.
int StreamReceiver::decodePacket(AVPacket* packet) {
    int ret = avcodec_send_packet(m_codecCtx, packet);
    if (ret < 0 && ret != AVERROR(EAGAIN)) {
        return -1;
    }

    int count = 0;
    while (true) {
        ret = avcodec_receive_frame(m_codecCtx, m_frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        }
        if (ret < 0) {
            return count > 0 ? count : -1;
        }

        VideoFrame outFrame;
        outFrame.image = convertToQImage(m_frame);
        outFrame.pts = m_frame->pts;
        outFrame.dts = m_frame->pkt_dts;
        outFrame.isKeyFrame = (m_frame->flags & AV_FRAME_FLAG_KEY) != 0;
        av_frame_unref(m_frame);

        if (outFrame.image.isNull()) {
            continue;
        }

        {
            QMutexLocker statsLock(&m_statsMutex);
            m_stats.framesDecoded++;
            m_fpsFrameCount++;
        }
        count++;
        emit frameReady(outFrame);
    }

    return count;
}

QImage StreamReceiver::convertToQImage(AVFrame* frame) {
    if (frame->width <= 0 || frame->height <= 0 || frame->format < 0) {
        return QImage();
    }

    AVPixelFormat format = static_cast<AVPixelFormat>(frame->format);
    if (!m_swsCtx || frame->width != m_swsWidth || frame->height != m_swsHeight || format != m_swsSrcFormat) {
        // 스트림 도중 해상도나 픽셀 포맷이 바뀌면 이전 크기로 만든 버퍼에 쓰면 안 된다.
        if (!initScaler(frame->width, frame->height, format)) {
            return QImage();
        }
    }

    sws_scale(m_swsCtx,
              frame->data, frame->linesize, 0, m_swsHeight,
              m_frameRGB->data, m_frameRGB->linesize);

    QImage img(m_frameRGB->data[0], m_swsWidth, m_swsHeight,
               m_frameRGB->linesize[0], QImage::Format_RGB32);

    return img.copy();
}

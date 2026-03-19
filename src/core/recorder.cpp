#include "recorder.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>

Recorder::Recorder(QObject* parent)
    : QObject(parent)
{
}

Recorder::~Recorder() {
    stopRecording();
}

bool Recorder::startRecording(const QString& cameraId, const QString& outputPath,
                               AVCodecParameters* videoParams, AVRational timeBase) {
    QMutexLocker locker(&m_mutex);
    
    if (m_recording) {
        return false;
    }
    
    m_cameraId = cameraId;
    m_currentFile = outputPath;
    
    QDir dir = QFileInfo(outputPath).absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    int ret = avformat_alloc_output_context2(&m_formatCtx, nullptr, nullptr, 
                                              outputPath.toUtf8().constData());
    if (ret < 0 || !m_formatCtx) {
        emit error("Failed to create output context");
        return false;
    }
    
    m_videoStream = avformat_new_stream(m_formatCtx, nullptr);
    if (!m_videoStream) {
        emit error("Failed to create video stream");
        cleanup();
        return false;
    }
    
    ret = avcodec_parameters_copy(m_videoStream->codecpar, videoParams);
    if (ret < 0) {
        emit error("Failed to copy codec parameters");
        cleanup();
        return false;
    }
    
    m_videoStream->time_base = timeBase;
    m_videoStream->codecpar->codec_tag = 0;
    
    if (!(m_formatCtx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&m_formatCtx->pb, outputPath.toUtf8().constData(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            emit error("Failed to open output file");
            cleanup();
            return false;
        }
    }
    
    ret = avformat_write_header(m_formatCtx, nullptr);
    if (ret < 0) {
        emit error("Failed to write header");
        cleanup();
        return false;
    }
    
    m_headerWritten = true;
    m_recording = true;
    m_startTime = QDateTime::currentDateTime();
    m_bytesWritten = 0;
    
    emit recordingStarted(outputPath);
    return true;
}

void Recorder::stopRecording() {
    QMutexLocker locker(&m_mutex);
    
    if (!m_recording) {
        return;
    }
    
    RecordingInfo info;
    info.cameraId = m_cameraId;
    info.filePath = m_currentFile;
    info.startTime = m_startTime;
    info.endTime = QDateTime::currentDateTime();
    info.fileSize = m_bytesWritten;
    
    if (m_formatCtx && m_headerWritten) {
        av_write_trailer(m_formatCtx);
    }
    
    cleanup();
    m_recording = false;
    m_headerWritten = false;
    
    emit recordingStopped(info);
}

bool Recorder::writePacket(AVPacket* packet) {
    QMutexLocker locker(&m_mutex);
    
    if (!m_recording || !m_formatCtx || !m_headerWritten) {
        return false;
    }
    
    AVPacket* pkt = av_packet_clone(packet);
    if (!pkt) {
        return false;
    }
    
    pkt->stream_index = m_videoStream->index;
    
    av_packet_rescale_ts(pkt, packet->time_base, m_videoStream->time_base);
    
    int ret = av_interleaved_write_frame(m_formatCtx, pkt);
    
    if (ret >= 0) {
        m_bytesWritten += pkt->size;
    }
    
    av_packet_free(&pkt);
    
    return ret >= 0;
}

void Recorder::cleanup() {
    if (m_formatCtx) {
        if (m_formatCtx->pb && !(m_formatCtx->oformat->flags & AVFMT_NOFILE)) {
            avio_closep(&m_formatCtx->pb);
        }
        avformat_free_context(m_formatCtx);
        m_formatCtx = nullptr;
    }
    m_videoStream = nullptr;
}

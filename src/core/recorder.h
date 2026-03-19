#ifndef RECORDER_H
#define RECORDER_H

#include <QObject>
#include <QString>
#include <QMutex>
#include <QDateTime>
#include <memory>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
}

struct RecordingInfo {
    QString id;
    QString cameraId;
    QString filePath;
    QDateTime startTime;
    QDateTime endTime;
    int64_t fileSize = 0;
    bool hasAudio = false;
};

class Recorder : public QObject {
    Q_OBJECT
    
public:
    explicit Recorder(QObject* parent = nullptr);
    ~Recorder();
    
    bool startRecording(const QString& cameraId, const QString& outputPath,
                        AVCodecParameters* videoParams, AVRational timeBase);
    void stopRecording();
    
    bool writePacket(AVPacket* packet);
    bool isRecording() const { return m_recording; }
    
    QString currentFile() const { return m_currentFile; }
    QDateTime startTime() const { return m_startTime; }
    int64_t bytesWritten() const { return m_bytesWritten; }
    
signals:
    void recordingStarted(const QString& filePath);
    void recordingStopped(const RecordingInfo& info);
    void error(const QString& message);
    
private:
    void cleanup();
    
    AVFormatContext* m_formatCtx = nullptr;
    AVStream* m_videoStream = nullptr;
    
    QString m_cameraId;
    QString m_currentFile;
    QDateTime m_startTime;
    int64_t m_bytesWritten = 0;
    bool m_recording = false;
    bool m_headerWritten = false;
    
    QMutex m_mutex;
};

#endif // RECORDER_H

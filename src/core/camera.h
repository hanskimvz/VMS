#ifndef CAMERA_H
#define CAMERA_H

#include <QString>
#include <QUuid>
#include <QDateTime>

enum class CameraType {
    ONVIF,
    RTSP,
    MJPEG,
    File
};

enum class CameraStatus {
    Offline,      // 메인·서브 스트림 모두 접속 실패
    Online,       // 둘 중 하나라도 접속 가능
    Connecting,   // 아직 확인 전
    Error
};

// 스트림 하나(메인 또는 서브)의 마지막 확인 결과
enum class StreamState {
    Unknown,
    Ok,
    Failed
};

struct CameraInfo {
    QString id;
    QString name;
    QString model;
    QString serialNumber;
    QString manufacturer;
    QString firmwareVersion;
    QString ip;
    int port = 80;
    QString username;
    QString password;
    QString rtspUrl;
    QString rtspUrlSub;
    QString onvifPath = "/onvif/device_service";
    CameraType type = CameraType::ONVIF;
    // status 는 아래 두 스트림 상태에서 deriveStatus() 로 계산된다. CameraManager 가 유지한다.
    CameraStatus status = CameraStatus::Connecting;
    StreamState mainStreamState = StreamState::Unknown;
    StreamState subStreamState = StreamState::Unknown;
    QString mainStreamError;
    QString subStreamError;
    bool recording = false;
    QDateTime lastSeen;
    
    static CameraInfo create(const QString& name, const QString& ip, const QString& model = QString()) {
        CameraInfo info;
        info.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        info.name = name;
        info.model = model;
        info.ip = ip;
        return info;
    }
    
    CameraStatus deriveStatus() const {
        if (mainStreamState == StreamState::Ok || subStreamState == StreamState::Ok) {
            return CameraStatus::Online;
        }
        if (mainStreamState == StreamState::Failed && subStreamState == StreamState::Failed) {
            return CameraStatus::Offline;
        }
        return CameraStatus::Connecting;
    }

    QString getOnvifUrl() const {
        return QString("http://%1:%2%3").arg(ip).arg(port).arg(onvifPath);
    }
    
    QString getDefaultRtspUrl() const {
        return QString("rtsp://%1:554/stream1").arg(ip);
    }
    
    QString getDefaultRtspUrlSub() const {
        return QString("rtsp://%1:554/stream2").arg(ip);
    }
    
    QString getRtspUrl(bool useSubStream) const {
        if (useSubStream) {
            return rtspUrlSub.isEmpty() ? getDefaultRtspUrlSub() : rtspUrlSub;
        }
        return rtspUrl.isEmpty() ? getDefaultRtspUrl() : rtspUrl;
    }
};

#endif // CAMERA_H

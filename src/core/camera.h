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
    Offline,
    Online,
    Connecting,
    Error
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
    CameraStatus status = CameraStatus::Offline;
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

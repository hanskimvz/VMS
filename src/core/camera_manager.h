#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

#include <QObject>
#include <QMap>
#include <QHash>
#include <QList>
#include <memory>
#include "camera.h"

class Database;
class StreamReceiver;

class CameraManager : public QObject {
    Q_OBJECT
    
public:
    explicit CameraManager(QObject* parent = nullptr);
    ~CameraManager();
    
    bool initialize(const QString& dbPath);
    
    QString addCamera(const CameraInfo& info);
    bool removeCamera(const QString& id);
    bool updateCamera(const CameraInfo& info);
    
    CameraInfo getCamera(const QString& id) const;
    QList<CameraInfo> getAllCameras() const;
    int cameraCount() const;
    
    bool startStream(const QString& id);
    bool stopStream(const QString& id);
    
    StreamReceiver* getStreamReceiver(const QString& id) const;
    
signals:
    void cameraAdded(const QString& id);
    void cameraRemoved(const QString& id);
    void cameraUpdated(const QString& id);
    void cameraStatusChanged(const QString& id, CameraStatus status);
    void streamStarted(const QString& id);
    void streamStopped(const QString& id);
    
private:
    void loadCamerasFromDb();
    void saveCameraToDb(const CameraInfo& info);
    void removeCameraFromDb(const QString& id);
    
    std::unique_ptr<Database> m_database;
    QMap<QString, CameraInfo> m_cameras;
    QHash<QString, StreamReceiver*> m_streamReceivers;
};

#endif // CAMERA_MANAGER_H

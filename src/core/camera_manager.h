#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

#include <QObject>
#include <QMap>
#include <QHash>
#include <QList>
#include <QTimer>
#include <memory>
#include "camera.h"

class Database;
class StreamReceiver;
class StreamHealthChecker;

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

    bool startStream(const QString& id, bool useSubStream = false);
    bool stopStream(const QString& id);
    bool restartStream(const QString& id, bool useSubStream);

    StreamReceiver* getStreamReceiver(const QString& id) const;
    bool isUsingSubStream(const QString& id) const;

    // 메인/서브 스트림을 백그라운드에서 열어 보고 CameraInfo 의 스트림 상태와 status 를 갱신한다.
    // 이미 라이브로 열려 있는 스트림은 다시 열지 않는다. 결과는 cameraStatusChanged 로 나온다.
    void checkCameraHealth(const QString& id);
    void checkAllCamerasHealth();

    static constexpr int HEALTH_CHECK_INTERVAL_MS = 60000;

signals:
    void cameraAdded(const QString& id);
    void cameraRemoved(const QString& id);
    void cameraUpdated(const QString& id);
    void cameraStatusChanged(const QString& id, CameraStatus status);
    void streamStarted(const QString& id);
    void streamStopped(const QString& id);
    void streamError(const QString& id, const QString& message);

private:
    void onReceiverDisconnected(const QString& id, StreamReceiver* receiver);
    void onStreamChecked(const QString& id, bool subStream, bool ok, const QString& error);
    void setStreamState(const QString& id, bool subStream, StreamState state, const QString& error);
    void loadCamerasFromDb();
    void saveCameraToDb(const CameraInfo& info);
    void removeCameraFromDb(const QString& id);

    std::unique_ptr<Database> m_database;
    QMap<QString, CameraInfo> m_cameras;
    QHash<QString, StreamReceiver*> m_streamReceivers;
    QHash<QString, bool> m_useSubStream;
    std::unique_ptr<StreamHealthChecker> m_healthChecker;
    QTimer m_healthTimer;
};

#endif // CAMERA_MANAGER_H

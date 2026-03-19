#include "camera_manager.h"
#include "database.h"
#include "stream_receiver.h"

CameraManager::CameraManager(QObject* parent)
    : QObject(parent)
    , m_database(std::make_unique<Database>())
{
}

CameraManager::~CameraManager() {
    for (auto it = m_streamReceivers.begin(); it != m_streamReceivers.end(); ++it) {
        if (it.value()) {
            it.value()->stop();
            delete it.value();
        }
    }
    m_streamReceivers.clear();
}

bool CameraManager::initialize(const QString& dbPath) {
    if (!m_database->open(dbPath)) {
        return false;
    }
    loadCamerasFromDb();
    return true;
}

QString CameraManager::addCamera(const CameraInfo& info) {
    CameraInfo newInfo = info;
    if (newInfo.id.isEmpty()) {
        newInfo.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
    
    m_cameras.insert(newInfo.id, newInfo);
    saveCameraToDb(newInfo);
    
    emit cameraAdded(newInfo.id);
    return newInfo.id;
}

bool CameraManager::removeCamera(const QString& id) {
    if (!m_cameras.contains(id)) {
        return false;
    }
    
    stopStream(id);
    m_cameras.remove(id);
    removeCameraFromDb(id);
    
    emit cameraRemoved(id);
    return true;
}

bool CameraManager::updateCamera(const CameraInfo& info) {
    if (!m_cameras.contains(info.id)) {
        return false;
    }
    
    m_cameras[info.id] = info;
    saveCameraToDb(info);
    
    emit cameraUpdated(info.id);
    return true;
}

CameraInfo CameraManager::getCamera(const QString& id) const {
    return m_cameras.value(id);
}

QList<CameraInfo> CameraManager::getAllCameras() const {
    return m_cameras.values();
}

int CameraManager::cameraCount() const {
    return m_cameras.count();
}

bool CameraManager::startStream(const QString& id) {
    if (!m_cameras.contains(id)) {
        return false;
    }
    
    if (m_streamReceivers.contains(id)) {
        return true;
    }
    
    const CameraInfo& info = m_cameras[id];
    
    StreamReceiver* receiver = new StreamReceiver();
    QString url = info.rtspUrl.isEmpty() ? info.getDefaultRtspUrl() : info.rtspUrl;
    
    if (!receiver->open(url, info.username, info.password)) {
        delete receiver;
        return false;
    }
    
    receiver->start();
    m_streamReceivers[id] = receiver;
    
    m_cameras[id].status = CameraStatus::Online;
    emit cameraStatusChanged(id, CameraStatus::Online);
    emit streamStarted(id);
    
    return true;
}

bool CameraManager::stopStream(const QString& id) {
    if (!m_streamReceivers.contains(id)) {
        return false;
    }
    
    StreamReceiver* receiver = m_streamReceivers.take(id);
    if (receiver) {
        receiver->stop();
        delete receiver;
    }
    
    if (m_cameras.contains(id)) {
        m_cameras[id].status = CameraStatus::Offline;
        emit cameraStatusChanged(id, CameraStatus::Offline);
    }
    emit streamStopped(id);
    
    return true;
}

StreamReceiver* CameraManager::getStreamReceiver(const QString& id) const {
    return m_streamReceivers.value(id, nullptr);
}

void CameraManager::loadCamerasFromDb() {
    QList<CameraInfo> cameras = m_database->loadCameras();
    for (const auto& cam : cameras) {
        m_cameras.insert(cam.id, cam);
    }
}

void CameraManager::saveCameraToDb(const CameraInfo& info) {
    m_database->saveCamera(info);
}

void CameraManager::removeCameraFromDb(const QString& id) {
    m_database->removeCamera(id);
}

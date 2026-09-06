#include "camera_manager.h"
#include "database.h"
#include "stream_receiver.h"
#include "stream_health_checker.h"
#include <QDebug>

CameraManager::CameraManager(QObject* parent)
    : QObject(parent)
    , m_database(std::make_unique<Database>())
    , m_healthChecker(std::make_unique<StreamHealthChecker>())
{
    connect(m_healthChecker.get(), &StreamHealthChecker::streamChecked,
            this, &CameraManager::onStreamChecked);

    m_healthTimer.setInterval(HEALTH_CHECK_INTERVAL_MS);
    connect(&m_healthTimer, &QTimer::timeout, this, &CameraManager::checkAllCamerasHealth);
}

CameraManager::~CameraManager() {
    m_healthTimer.stop();
    m_healthChecker.reset();   // 진행 중인 프로브를 중단하고 기다린다.

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
    checkAllCamerasHealth();
    m_healthTimer.start();
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
    checkCameraHealth(newInfo.id);
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

    // URL 이나 자격 증명이 바뀌었을 수 있으므로 이전 확인 결과는 버리고 다시 확인한다.
    CameraInfo updated = info;
    updated.mainStreamState = StreamState::Unknown;
    updated.subStreamState = StreamState::Unknown;
    updated.mainStreamError.clear();
    updated.subStreamError.clear();
    updated.status = updated.deriveStatus();

    m_cameras[info.id] = updated;
    saveCameraToDb(updated);

    emit cameraUpdated(info.id);
    checkCameraHealth(info.id);
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

bool CameraManager::startStream(const QString& id, bool useSubStream) {
    if (!m_cameras.contains(id)) {
        return false;
    }

    if (m_streamReceivers.contains(id)) {
        if (m_useSubStream.value(id, false) == useSubStream) {
            return true;
        }
        stopStream(id);
    }

    const CameraInfo& info = m_cameras[id];

    StreamReceiver* receiver = new StreamReceiver();
    QString url = info.getRtspUrl(useSubStream);

    qDebug() << "Starting stream for" << info.name << "using"
             << (useSubStream ? "sub-stream" : "main-stream") << ":" << url;

    // error 는 open() 안에서도(호출 스레드) 나오고 run() 안에서도(워커 스레드) 나온다.
    // open() 전에 AutoConnection 으로 연결해야 열기 실패 사유가 로그와 상태바에 남는다.
    connect(receiver, &StreamReceiver::error, this,
            [this, id](const QString& message) { emit streamError(id, message); });

    // 열기 실패 사유를 스트림 상태에 남기기 위한 임시 연결. open() 이 끝나면 끊는다.
    QString openError;
    QMetaObject::Connection errorCapture = connect(receiver, &StreamReceiver::error,
            [&openError](const QString& message) { openError = message; });
    bool opened = receiver->open(url, info.username, info.password);
    disconnect(errorCapture);

    if (!opened) {
        qWarning() << "Failed to open" << (useSubStream ? "sub-stream" : "main-stream")
                   << "for" << info.name << ":" << openError;
        delete receiver;
        setStreamState(id, useSubStream, StreamState::Failed, openError);
        return false;
    }

    // 워커 스레드에서 오는 시그널이므로 큐드 커넥션으로 GUI 스레드에서 처리한다.
    // receiver 포인터는 "아직 같은 리시버인지" 비교에만 쓰고 역참조하지 않는다.
    connect(receiver, &StreamReceiver::disconnected, this,
            [this, id, receiver]() { onReceiverDisconnected(id, receiver); },
            Qt::QueuedConnection);

    receiver->start();
    m_streamReceivers[id] = receiver;
    m_useSubStream[id] = useSubStream;

    setStreamState(id, useSubStream, StreamState::Ok, QString());
    emit streamStarted(id);

    return true;
}

bool CameraManager::restartStream(const QString& id, bool useSubStream) {
    if (!m_cameras.contains(id)) {
        return false;
    }

    if (!m_streamReceivers.contains(id)) {
        return startStream(id, useSubStream);
    }

    if (m_useSubStream.value(id, false) == useSubStream) {
        return true;
    }

    stopStream(id);
    return startStream(id, useSubStream);
}

bool CameraManager::isUsingSubStream(const QString& id) const {
    return m_useSubStream.value(id, false);
}

bool CameraManager::stopStream(const QString& id) {
    if (!m_streamReceivers.contains(id)) {
        return false;
    }

    StreamReceiver* receiver = m_streamReceivers.take(id);
    m_useSubStream.remove(id);

    if (receiver) {
        receiver->stop();
        // VideoWidget 등이 아직 포인터를 들고 있을 수 있으므로 이벤트 루프에서 지운다.
        // 참조하는 쪽은 QPointer 를 쓰고, streamStopped 로 정리 기회를 받는다.
        receiver->deleteLater();
    }

    // 라이브 뷰를 닫는 것은 카메라가 사라진 것이 아니다. 스트림 상태(Online/Offline)는 건드리지 않는다.
    emit streamStopped(id);

    return true;
}

void CameraManager::onReceiverDisconnected(const QString& id, StreamReceiver* receiver) {
    // 이미 stopStream 으로 정리됐거나 다른 리시버로 교체된 뒤 도착한 신호는 무시한다.
    if (m_streamReceivers.value(id, nullptr) != receiver) {
        return;
    }
    qWarning() << "Stream disconnected:" << id;
    bool wasSub = m_useSubStream.value(id, false);
    stopStream(id);
    setStreamState(id, wasSub, StreamState::Failed, "Disconnected");
    // 카메라 전체가 사라진 것인지 그 스트림만 문제인지는 다시 열어 봐야 안다.
    checkCameraHealth(id);
}

void CameraManager::checkCameraHealth(const QString& id) {
    if (!m_cameras.contains(id)) {
        return;
    }
    const CameraInfo& info = m_cameras[id];
    bool liveRunning = m_streamReceivers.contains(id);
    bool liveIsSub = m_useSubStream.value(id, false);

    for (bool sub : {false, true}) {
        // 라이브로 이미 열려 있는 스트림은 startStream 에서 Ok 로 기록됐으므로 다시 열지 않는다.
        if (liveRunning && liveIsSub == sub) {
            continue;
        }
        m_healthChecker->check(id, sub, info.getRtspUrl(sub), info.username, info.password);
    }
}

void CameraManager::checkAllCamerasHealth() {
    for (auto it = m_cameras.constBegin(); it != m_cameras.constEnd(); ++it) {
        checkCameraHealth(it.key());
    }
}

void CameraManager::onStreamChecked(const QString& id, bool subStream, bool ok, const QString& error) {
    if (!m_cameras.contains(id)) {
        return;   // 확인 중에 삭제된 카메라
    }
    // 확인하는 사이에 같은 스트림이 라이브로 열렸으면 그쪽 결과가 더 최신이다.
    if (m_streamReceivers.contains(id) && m_useSubStream.value(id, false) == subStream) {
        return;
    }
    const CameraInfo& info = m_cameras[id];
    if (ok) {
        qDebug() << "Health check OK:" << info.name << info.ip << (subStream ? "sub-stream" : "main-stream");
    } else {
        qWarning() << "Health check failed:" << info.name << info.ip
                   << (subStream ? "sub-stream" : "main-stream") << ":" << error;
    }
    setStreamState(id, subStream, ok ? StreamState::Ok : StreamState::Failed, ok ? QString() : error);
}

void CameraManager::setStreamState(const QString& id, bool subStream, StreamState state, const QString& error) {
    if (!m_cameras.contains(id)) {
        return;
    }
    CameraInfo& info = m_cameras[id];
    StreamState& target = subStream ? info.subStreamState : info.mainStreamState;
    QString& targetError = subStream ? info.subStreamError : info.mainStreamError;

    bool changed = (target != state) || (targetError != error);
    target = state;
    targetError = error;

    CameraStatus newStatus = info.deriveStatus();
    changed = changed || (info.status != newStatus);
    info.status = newStatus;

    // 개별 스트림 결과가 바뀌면 전체 status 가 같아도 표(장치 관리)는 다시 그려야 한다.
    if (changed) {
        emit cameraStatusChanged(id, newStatus);
    }
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

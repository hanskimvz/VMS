#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QUuid>
#include <QDir>
#include <QFileInfo>

Database::Database() {
    m_connectionName = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

Database::~Database() {
    close();
}

bool Database::open(const QString& dbPath) {
    QDir dir = QFileInfo(dbPath).absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_db.setDatabaseName(dbPath);
    
    if (!m_db.open()) {
        qWarning() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }
    
    return createTables();
}

void Database::close() {
    if (m_db.isOpen()) {
        m_db.close();
    }
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool Database::isOpen() const {
    return m_db.isOpen();
}

bool Database::createTables() {
    QSqlQuery query(m_db);
    
    bool success = query.exec(
        "CREATE TABLE IF NOT EXISTS cameras ("
        "id TEXT PRIMARY KEY,"
        "name TEXT NOT NULL,"
        "ip TEXT NOT NULL,"
        "port INTEGER DEFAULT 80,"
        "username TEXT,"
        "password TEXT,"
        "rtsp_url TEXT,"
        "rtsp_url_sub TEXT,"
        "onvif_path TEXT DEFAULT '/onvif/device_service',"
        "type INTEGER DEFAULT 0,"
        "recording INTEGER DEFAULT 0,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ")"
    );
    
    if (!success) {
        qWarning() << "Failed to create cameras table:" << query.lastError().text();
        return false;
    }
    
    success = query.exec(
        "CREATE TABLE IF NOT EXISTS recordings ("
        "id TEXT PRIMARY KEY,"
        "camera_id TEXT NOT NULL,"
        "file_path TEXT NOT NULL,"
        "start_time DATETIME NOT NULL,"
        "end_time DATETIME,"
        "file_size INTEGER DEFAULT 0,"
        "has_audio INTEGER DEFAULT 0,"
        "FOREIGN KEY (camera_id) REFERENCES cameras(id)"
        ")"
    );
    
    if (!success) {
        qWarning() << "Failed to create recordings table:" << query.lastError().text();
        return false;
    }
    
    query.exec("CREATE INDEX IF NOT EXISTS idx_recordings_camera ON recordings(camera_id)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_recordings_time ON recordings(start_time, end_time)");
    
    return true;
}

bool Database::saveCamera(const CameraInfo& camera) {
    QSqlQuery query(m_db);
    
    query.prepare(
        "INSERT OR REPLACE INTO cameras "
        "(id, name, ip, port, username, password, rtsp_url, rtsp_url_sub, onvif_path, type, recording) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
    );
    
    query.addBindValue(camera.id);
    query.addBindValue(camera.name);
    query.addBindValue(camera.ip);
    query.addBindValue(camera.port);
    query.addBindValue(camera.username);
    query.addBindValue(camera.password);
    query.addBindValue(camera.rtspUrl);
    query.addBindValue(camera.rtspUrlSub);
    query.addBindValue(camera.onvifPath);
    query.addBindValue(static_cast<int>(camera.type));
    query.addBindValue(camera.recording ? 1 : 0);
    
    if (!query.exec()) {
        qWarning() << "Failed to save camera:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool Database::removeCamera(const QString& id) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM cameras WHERE id = ?");
    query.addBindValue(id);
    
    return query.exec();
}

QList<CameraInfo> Database::loadCameras() {
    QList<CameraInfo> cameras;
    
    QSqlQuery query(m_db);
    query.exec("SELECT * FROM cameras");
    
    while (query.next()) {
        CameraInfo camera;
        camera.id = query.value("id").toString();
        camera.name = query.value("name").toString();
        camera.ip = query.value("ip").toString();
        camera.port = query.value("port").toInt();
        camera.username = query.value("username").toString();
        camera.password = query.value("password").toString();
        camera.rtspUrl = query.value("rtsp_url").toString();
        camera.rtspUrlSub = query.value("rtsp_url_sub").toString();
        camera.onvifPath = query.value("onvif_path").toString();
        camera.type = static_cast<CameraType>(query.value("type").toInt());
        camera.recording = query.value("recording").toBool();
        
        cameras.append(camera);
    }
    
    return cameras;
}

CameraInfo Database::loadCamera(const QString& id) {
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM cameras WHERE id = ?");
    query.addBindValue(id);
    
    if (query.exec() && query.next()) {
        CameraInfo camera;
        camera.id = query.value("id").toString();
        camera.name = query.value("name").toString();
        camera.ip = query.value("ip").toString();
        camera.port = query.value("port").toInt();
        camera.username = query.value("username").toString();
        camera.password = query.value("password").toString();
        camera.rtspUrl = query.value("rtsp_url").toString();
        camera.rtspUrlSub = query.value("rtsp_url_sub").toString();
        camera.onvifPath = query.value("onvif_path").toString();
        camera.type = static_cast<CameraType>(query.value("type").toInt());
        camera.recording = query.value("recording").toBool();
        
        return camera;
    }
    
    return CameraInfo();
}

bool Database::saveRecording(const RecordingInfo& recording) {
    QSqlQuery query(m_db);
    
    query.prepare(
        "INSERT OR REPLACE INTO recordings "
        "(id, camera_id, file_path, start_time, end_time, file_size, has_audio) "
        "VALUES (?, ?, ?, ?, ?, ?, ?)"
    );
    
    query.addBindValue(recording.id);
    query.addBindValue(recording.cameraId);
    query.addBindValue(recording.filePath);
    query.addBindValue(recording.startTime);
    query.addBindValue(recording.endTime);
    query.addBindValue(recording.fileSize);
    query.addBindValue(recording.hasAudio ? 1 : 0);
    
    return query.exec();
}

bool Database::removeRecording(const QString& id) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM recordings WHERE id = ?");
    query.addBindValue(id);
    
    return query.exec();
}

QList<RecordingInfo> Database::searchRecordings(const QString& cameraId,
                                                  const QDateTime& startTime,
                                                  const QDateTime& endTime) {
    QList<RecordingInfo> recordings;
    
    QSqlQuery query(m_db);
    query.prepare(
        "SELECT * FROM recordings "
        "WHERE camera_id = ? AND start_time >= ? AND end_time <= ? "
        "ORDER BY start_time"
    );
    query.addBindValue(cameraId);
    query.addBindValue(startTime);
    query.addBindValue(endTime);
    
    if (query.exec()) {
        while (query.next()) {
            RecordingInfo rec;
            rec.id = query.value("id").toString();
            rec.cameraId = query.value("camera_id").toString();
            rec.filePath = query.value("file_path").toString();
            rec.startTime = query.value("start_time").toDateTime();
            rec.endTime = query.value("end_time").toDateTime();
            rec.fileSize = query.value("file_size").toLongLong();
            rec.hasAudio = query.value("has_audio").toBool();
            
            recordings.append(rec);
        }
    }
    
    return recordings;
}

QList<RecordingInfo> Database::getRecordingsForCamera(const QString& cameraId) {
    QList<RecordingInfo> recordings;
    
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM recordings WHERE camera_id = ? ORDER BY start_time DESC");
    query.addBindValue(cameraId);
    
    if (query.exec()) {
        while (query.next()) {
            RecordingInfo rec;
            rec.id = query.value("id").toString();
            rec.cameraId = query.value("camera_id").toString();
            rec.filePath = query.value("file_path").toString();
            rec.startTime = query.value("start_time").toDateTime();
            rec.endTime = query.value("end_time").toDateTime();
            rec.fileSize = query.value("file_size").toLongLong();
            rec.hasAudio = query.value("has_audio").toBool();
            
            recordings.append(rec);
        }
    }
    
    return recordings;
}

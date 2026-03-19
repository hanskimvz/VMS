#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include <QList>
#include <QSqlDatabase>
#include "camera.h"
#include "recorder.h"

class Database {
public:
    Database();
    ~Database();
    
    bool open(const QString& dbPath);
    void close();
    bool isOpen() const;
    
    bool saveCamera(const CameraInfo& camera);
    bool removeCamera(const QString& id);
    QList<CameraInfo> loadCameras();
    CameraInfo loadCamera(const QString& id);
    
    bool saveRecording(const RecordingInfo& recording);
    bool removeRecording(const QString& id);
    QList<RecordingInfo> searchRecordings(const QString& cameraId,
                                           const QDateTime& startTime,
                                           const QDateTime& endTime);
    QList<RecordingInfo> getRecordingsForCamera(const QString& cameraId);
    
private:
    bool createTables();
    
    QSqlDatabase m_db;
    QString m_connectionName;
};

#endif // DATABASE_H

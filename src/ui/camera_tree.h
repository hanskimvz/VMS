#ifndef CAMERA_TREE_H
#define CAMERA_TREE_H

#include <QTreeWidget>
#include <QMap>
#include <QTimer>
#include "camera.h"

class CameraManager;

class CameraTree : public QTreeWidget {
    Q_OBJECT
    
public:
    explicit CameraTree(QWidget* parent = nullptr);
    ~CameraTree();
    
    void setCameraManager(CameraManager* manager);
    
public slots:
    void refreshCameras();
    void checkCameraStatus();
    void updateCameraStatus(const QString& cameraId, bool isOnline);
    
signals:
    void cameraSelected(const QString& cameraId);
    void cameraDoubleClicked(const QString& cameraId);
    void cameraContextMenu(const QString& cameraId, const QPoint& pos);
    // 컨텍스트 메뉴의 Start Stream / Edit. 실제 처리는 MainWindow 가 한다(그리드 배치, 대화상자).
    void streamStartRequested(const QString& cameraId);
    void cameraEditRequested(const QString& cameraId);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

private slots:
    void onItemSelectionChanged();
    void onCameraStatusChanged(const QString& cameraId, CameraStatus status);
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);
    void onDeleteCamera();
    void onEditCamera();
    void onStartStream();
    void onStopStream();
    
private:
    void setupUi();
    void addCameraItem(const CameraInfo& camera);
    QString getSelectedCameraId() const;
    QIcon createStatusIcon(bool isOnline) const;
    
    CameraManager* m_cameraManager = nullptr;
    QMap<QString, QTreeWidgetItem*> m_cameraItems;
    QMap<QString, bool> m_cameraStatusCache;
    
    QTreeWidgetItem* m_defaultGroupItem = nullptr;
    QTimer* m_statusCheckTimer = nullptr;
};

#endif // CAMERA_TREE_H

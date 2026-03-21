#ifndef CAMERA_TREE_H
#define CAMERA_TREE_H

#include <QTreeWidget>
#include <QMap>
#include <QTimer>

class CameraManager;
struct CameraInfo;

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
    
protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    
private slots:
    void onItemSelectionChanged();
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

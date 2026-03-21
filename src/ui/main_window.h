#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QTreeWidget>
#include <QToolButton>
#include <QLabel>
#include <QSplitter>
#include <memory>

class CameraManager;
class OnvifClient;
class VideoGrid;
class CameraTree;
class PlaybackView;
class DeviceManageWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    
private slots:
    void onAddCamera();
    void onDiscoverCameras();
    void onCameraSelected(const QString& cameraId);
    void onCameraDoubleClicked(const QString& cameraId);
    void onLayoutChanged(int layout);
    void onShowLiveView();
    void onShowDeviceManage();
    void onShowPlayback();
    void onShowRecordSchedule();
    void onShowSettings();
    void onAbout();
    
private:
    void setupUi();
    void setupHeaderBar();
    void setupSidePanel();
    void setupCentralArea();
    void setupLayoutButtons();
    void setupConnections();
    void loadSettings();
    void saveSettings();
    void applyDarkTheme();
    void setActiveTab(QToolButton* button);
    bool shouldUseSubStream() const;
    void updateStreamsForLayout();
    
    std::unique_ptr<CameraManager> m_cameraManager;
    std::unique_ptr<OnvifClient> m_onvifClient;
    
    // Header buttons
    QToolButton* m_liveBtn = nullptr;
    QToolButton* m_deviceManageBtn = nullptr;
    QToolButton* m_playbackBtn = nullptr;
    QToolButton* m_recordScheduleBtn = nullptr;
    QToolButton* m_settingsBtn = nullptr;
    QVector<QToolButton*> m_tabButtons;
    
    // Main areas
    QStackedWidget* m_stackedWidget = nullptr;
    VideoGrid* m_liveGrid = nullptr;
    PlaybackView* m_playbackView = nullptr;
    DeviceManageWidget* m_deviceManageWidget = nullptr;
    QWidget* m_recordSchedulePage = nullptr;
    QWidget* m_settingsPage = nullptr;
    
    // Side panel
    CameraTree* m_cameraTree = nullptr;
    
    // Layout buttons
    QWidget* m_layoutBar = nullptr;
    
    // Status
    QLabel* m_statusLabel = nullptr;
    
    QString m_selectedCameraId;
};

#endif // MAIN_WINDOW_H

#ifndef PLAYBACK_VIEW_H
#define PLAYBACK_VIEW_H

#include <QWidget>
#include <QSplitter>
#include <memory>

class VideoWidget;
class TimelineWidget;
class PlaybackController;
class QPushButton;
class QSlider;
class QLabel;
class QDateTimeEdit;
class QComboBox;

class PlaybackView : public QWidget {
    Q_OBJECT
    
public:
    explicit PlaybackView(QWidget* parent = nullptr);
    ~PlaybackView();
    
    void setCameraId(const QString& cameraId);
    void loadRecording(const QString& filePath);
    
signals:
    void recordingSelected(const QString& filePath);
    
private slots:
    void onPlay();
    void onPause();
    void onStop();
    void onSeek(int64_t timestamp);
    void onSpeedChanged(int index);
    void onPositionChanged(int64_t position);
    void onStateChanged(int state);
    void onTimelineClicked(int64_t timestamp);
    
private:
    void setupUi();
    void setupConnections();
    void updatePlayButton();
    void updateTimeDisplay(int64_t position);
    
    std::unique_ptr<PlaybackController> m_controller;
    
    VideoWidget* m_videoWidget = nullptr;
    TimelineWidget* m_timeline = nullptr;
    
    QPushButton* m_playButton = nullptr;
    QPushButton* m_stopButton = nullptr;
    QSlider* m_seekSlider = nullptr;
    QLabel* m_timeLabel = nullptr;
    QLabel* m_durationLabel = nullptr;
    QComboBox* m_speedCombo = nullptr;
    QDateTimeEdit* m_dateTimeEdit = nullptr;
    
    QString m_currentCameraId;
    QString m_currentFile;
};

#endif // PLAYBACK_VIEW_H

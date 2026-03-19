#ifndef VIDEO_WIDGET_H
#define VIDEO_WIDGET_H

#include <QWidget>
#include <QImage>
#include <QMutex>
#include <QLabel>
#include <QTimer>

class StreamReceiver;
struct VideoFrame;

class VideoWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit VideoWidget(QWidget* parent = nullptr);
    ~VideoWidget();
    
    void setStreamReceiver(StreamReceiver* receiver);
    void removeStreamReceiver();
    
    void setCameraId(const QString& id) { m_cameraId = id; }
    void setCameraName(const QString& name);
    QString cameraId() const { return m_cameraId; }
    QString cameraName() const { return m_cameraName; }
    
    bool isPlaying() const { return m_receiver != nullptr; }
    
    void setSelected(bool selected);
    bool isSelected() const { return m_selected; }
    
signals:
    void clicked(const QString& cameraId);
    void doubleClicked(const QString& cameraId);
    void contextMenuRequested(const QString& cameraId, const QPoint& pos);
    
public slots:
    void displayFrame(const QImage& image);
    
protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    
private slots:
    void onFrameReady(const VideoFrame& frame);
    void updateDisplay();
    
private:
    void drawNoSignal(QPainter& painter);
    void drawCameraName(QPainter& painter);
    
    StreamReceiver* m_receiver = nullptr;
    
    QString m_cameraId;
    QString m_cameraName;
    
    QImage m_currentFrame;
    QImage m_scaledFrame;
    QMutex m_frameMutex;
    
    QTimer* m_updateTimer;
    bool m_frameReady = false;
    bool m_selected = false;
    
    static const int UPDATE_INTERVAL_MS = 33;
};

#endif // VIDEO_WIDGET_H

#ifndef VIDEO_GRID_H
#define VIDEO_GRID_H

#include <QWidget>
#include <QGridLayout>
#include <QVector>
#include <QMap>
#include <QPointer>

class VideoWidget;
class StreamReceiver;

struct StreamInfo {
    QString cameraId;
    QString name;
    QPointer<StreamReceiver> receiver;   // CameraManager 소유. 삭제되면 자동으로 null
    int slotIndex = -1;
};

class VideoGrid : public QWidget {
    Q_OBJECT
    
public:
    explicit VideoGrid(QWidget* parent = nullptr);
    ~VideoGrid();
    
    void setLayout(int cellCount);
    int layout() const { return m_cellCount; }
    
    void addStream(const QString& cameraId, const QString& name, StreamReceiver* receiver);
    void removeStream(const QString& cameraId);
    void removeAllStreams();
    
    VideoWidget* getWidget(int index) const;
    VideoWidget* getWidgetByCameraId(const QString& cameraId) const;
    
    int findEmptySlot() const;
    int widgetCount() const { return m_widgets.size(); }
    int streamCount() const { return m_streams.size(); }
    
    bool isMaximized() const { return m_isMaximized; }
    void toggleMaximize(int widgetIndex);
    void toggleStats();
    
signals:
    void widgetClicked(int index, const QString& cameraId);
    void widgetDoubleClicked(int index, const QString& cameraId);
    void contextMenuRequested(int index, const QString& cameraId, const QPoint& pos);
    void layoutChanged(int cellCount);
    
private slots:
    void onWidgetClicked(const QString& cameraId);
    void onWidgetDoubleClicked(const QString& cameraId);
    void onContextMenuRequested(const QString& cameraId, const QPoint& pos);
    
private:
    void initLayout(int cellCount);
    void createWidgets(int count);
    void arrangeWidgets();
    void arrangeWidgetsMaximized(int focusIndex);
    void assignStreamsToWidgets();
    void clearSelection();
    int getWidgetIndex(const QString& cameraId) const;
    
    QGridLayout* m_gridLayout;
    QVector<VideoWidget*> m_widgets;
    QMap<QString, StreamInfo> m_streams;
    
    int m_cellCount = 4;
    int m_selectedIndex = -1;
    
    bool m_isMaximized = false;
    int m_previousLayout = 4;
    int m_maximizedWidgetIndex = -1;
    bool m_showStats = false;
};

#endif // VIDEO_GRID_H

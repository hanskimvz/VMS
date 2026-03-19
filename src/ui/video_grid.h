#ifndef VIDEO_GRID_H
#define VIDEO_GRID_H

#include <QWidget>
#include <QGridLayout>
#include <QVector>
#include <QMap>

class VideoWidget;
class StreamReceiver;

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
    
signals:
    void widgetClicked(int index, const QString& cameraId);
    void widgetDoubleClicked(int index, const QString& cameraId);
    void contextMenuRequested(int index, const QString& cameraId, const QPoint& pos);
    
private slots:
    void onWidgetClicked(const QString& cameraId);
    void onWidgetDoubleClicked(const QString& cameraId);
    void onContextMenuRequested(const QString& cameraId, const QPoint& pos);
    
private:
    void createWidgets(int count);
    void arrangeWidgets();
    void clearSelection();
    int getWidgetIndex(const QString& cameraId) const;
    
    QGridLayout* m_gridLayout;
    QVector<VideoWidget*> m_widgets;
    QMap<QString, int> m_cameraToWidget;
    
    int m_cellCount = 4;
    int m_selectedIndex = -1;
};

#endif // VIDEO_GRID_H

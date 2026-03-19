#ifndef TIMELINE_WIDGET_H
#define TIMELINE_WIDGET_H

#include <QWidget>
#include <QDateTime>
#include <QVector>
#include <QPair>

struct RecordingSegment {
    int64_t startTime;
    int64_t endTime;
    bool hasAlarm = false;
};

class TimelineWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit TimelineWidget(QWidget* parent = nullptr);
    ~TimelineWidget();
    
    void setDuration(int64_t duration);
    void setCurrentPosition(int64_t position);
    void setRecordingSegments(const QVector<RecordingSegment>& segments);
    void setStartTime(const QDateTime& startTime);
    
    void setZoomLevel(double level);
    void zoomIn();
    void zoomOut();
    
    int64_t duration() const { return m_duration; }
    int64_t currentPosition() const { return m_currentPosition; }
    
signals:
    void positionClicked(int64_t timestamp);
    void rangeSelected(int64_t start, int64_t end);
    
protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    
private:
    void drawBackground(QPainter& painter);
    void drawTimeScale(QPainter& painter);
    void drawRecordingSegments(QPainter& painter);
    void drawCurrentPosition(QPainter& painter);
    void drawHoverPosition(QPainter& painter);
    
    int64_t positionToTimestamp(int x) const;
    int timestampToPosition(int64_t timestamp) const;
    QString formatTime(int64_t timestamp) const;
    
    int64_t m_duration = 0;
    int64_t m_currentPosition = 0;
    int64_t m_viewStart = 0;
    int64_t m_viewDuration = 0;
    
    QDateTime m_startTime;
    QVector<RecordingSegment> m_segments;
    
    double m_zoomLevel = 1.0;
    bool m_dragging = false;
    int m_dragStartX = 0;
    int64_t m_dragStartViewStart = 0;
    
    int m_hoverX = -1;
    
    static constexpr int MARGIN_LEFT = 50;
    static constexpr int MARGIN_RIGHT = 20;
    static constexpr int MARGIN_TOP = 20;
    static constexpr int MARGIN_BOTTOM = 25;
};

#endif // TIMELINE_WIDGET_H

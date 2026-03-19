#include "timeline_widget.h"

#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <cmath>

TimelineWidget::TimelineWidget(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setMinimumHeight(60);
    
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(30, 30, 30));
    setPalette(pal);
    setAutoFillBackground(true);
}

TimelineWidget::~TimelineWidget() {
}

void TimelineWidget::setDuration(int64_t duration) {
    m_duration = duration;
    m_viewDuration = duration;
    m_viewStart = 0;
    update();
}

void TimelineWidget::setCurrentPosition(int64_t position) {
    m_currentPosition = position;
    
    if (position < m_viewStart || position > m_viewStart + m_viewDuration) {
        m_viewStart = qMax(0LL, position - m_viewDuration / 4);
    }
    
    update();
}

void TimelineWidget::setRecordingSegments(const QVector<RecordingSegment>& segments) {
    m_segments = segments;
    update();
}

void TimelineWidget::setStartTime(const QDateTime& startTime) {
    m_startTime = startTime;
    update();
}

void TimelineWidget::setZoomLevel(double level) {
    m_zoomLevel = qBound(0.1, level, 10.0);
    
    int64_t center = m_viewStart + m_viewDuration / 2;
    m_viewDuration = static_cast<int64_t>(m_duration / m_zoomLevel);
    m_viewStart = qMax(0LL, center - m_viewDuration / 2);
    
    if (m_viewStart + m_viewDuration > m_duration) {
        m_viewStart = m_duration - m_viewDuration;
    }
    
    update();
}

void TimelineWidget::zoomIn() {
    setZoomLevel(m_zoomLevel * 1.5);
}

void TimelineWidget::zoomOut() {
    setZoomLevel(m_zoomLevel / 1.5);
}

void TimelineWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    drawBackground(painter);
    drawTimeScale(painter);
    drawRecordingSegments(painter);
    drawCurrentPosition(painter);
    
    if (m_hoverX >= 0) {
        drawHoverPosition(painter);
    }
}

void TimelineWidget::drawBackground(QPainter& painter) {
    QRect timelineRect(MARGIN_LEFT, MARGIN_TOP, 
                       width() - MARGIN_LEFT - MARGIN_RIGHT,
                       height() - MARGIN_TOP - MARGIN_BOTTOM);
    
    painter.fillRect(timelineRect, QColor(45, 45, 48));
    painter.setPen(QColor(80, 80, 80));
    painter.drawRect(timelineRect);
}

void TimelineWidget::drawTimeScale(QPainter& painter) {
    if (m_viewDuration <= 0) return;
    
    painter.setPen(QColor(150, 150, 150));
    painter.setFont(QFont("Arial", 8));
    
    int timelineWidth = width() - MARGIN_LEFT - MARGIN_RIGHT;
    
    int64_t interval = 1000;
    if (m_viewDuration > 60000) interval = 10000;
    if (m_viewDuration > 600000) interval = 60000;
    if (m_viewDuration > 3600000) interval = 600000;
    
    int64_t start = (m_viewStart / interval) * interval;
    
    for (int64_t t = start; t <= m_viewStart + m_viewDuration; t += interval) {
        int x = timestampToPosition(t);
        
        if (x >= MARGIN_LEFT && x <= width() - MARGIN_RIGHT) {
            painter.drawLine(x, MARGIN_TOP, x, MARGIN_TOP + 5);
            
            QString timeStr = formatTime(t);
            QRect textRect = painter.fontMetrics().boundingRect(timeStr);
            painter.drawText(x - textRect.width() / 2, height() - 5, timeStr);
        }
    }
}

void TimelineWidget::drawRecordingSegments(QPainter& painter) {
    if (m_segments.isEmpty() && m_duration > 0) {
        int x1 = timestampToPosition(0);
        int x2 = timestampToPosition(m_duration);
        
        QRect segmentRect(x1, MARGIN_TOP + 10, x2 - x1, height() - MARGIN_TOP - MARGIN_BOTTOM - 20);
        painter.fillRect(segmentRect, QColor(0, 120, 215, 150));
        return;
    }
    
    for (const RecordingSegment& segment : m_segments) {
        int x1 = timestampToPosition(segment.startTime);
        int x2 = timestampToPosition(segment.endTime);
        
        QRect segmentRect(x1, MARGIN_TOP + 10, x2 - x1, height() - MARGIN_TOP - MARGIN_BOTTOM - 20);
        
        QColor color = segment.hasAlarm ? QColor(220, 50, 50, 150) : QColor(0, 120, 215, 150);
        painter.fillRect(segmentRect, color);
    }
}

void TimelineWidget::drawCurrentPosition(QPainter& painter) {
    int x = timestampToPosition(m_currentPosition);
    
    if (x >= MARGIN_LEFT && x <= width() - MARGIN_RIGHT) {
        painter.setPen(QPen(QColor(255, 200, 0), 2));
        painter.drawLine(x, MARGIN_TOP, x, height() - MARGIN_BOTTOM);
        
        QPolygon triangle;
        triangle << QPoint(x - 5, MARGIN_TOP - 2)
                 << QPoint(x + 5, MARGIN_TOP - 2)
                 << QPoint(x, MARGIN_TOP + 5);
        painter.setBrush(QColor(255, 200, 0));
        painter.drawPolygon(triangle);
    }
}

void TimelineWidget::drawHoverPosition(QPainter& painter) {
    if (m_hoverX < MARGIN_LEFT || m_hoverX > width() - MARGIN_RIGHT) {
        return;
    }
    
    painter.setPen(QPen(QColor(255, 255, 255, 100), 1, Qt::DashLine));
    painter.drawLine(m_hoverX, MARGIN_TOP, m_hoverX, height() - MARGIN_BOTTOM);
    
    int64_t timestamp = positionToTimestamp(m_hoverX);
    QString timeStr = formatTime(timestamp);
    
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 9));
    
    QRect textRect = painter.fontMetrics().boundingRect(timeStr);
    int textX = qBound(MARGIN_LEFT, m_hoverX - textRect.width() / 2, 
                       width() - MARGIN_RIGHT - textRect.width());
    
    painter.fillRect(textX - 2, MARGIN_TOP - 18, textRect.width() + 4, textRect.height() + 2,
                     QColor(0, 0, 0, 180));
    painter.drawText(textX, MARGIN_TOP - 5, timeStr);
}

int64_t TimelineWidget::positionToTimestamp(int x) const {
    if (m_viewDuration <= 0) return 0;
    
    int timelineWidth = width() - MARGIN_LEFT - MARGIN_RIGHT;
    double ratio = static_cast<double>(x - MARGIN_LEFT) / timelineWidth;
    
    return m_viewStart + static_cast<int64_t>(ratio * m_viewDuration);
}

int TimelineWidget::timestampToPosition(int64_t timestamp) const {
    if (m_viewDuration <= 0) return MARGIN_LEFT;
    
    int timelineWidth = width() - MARGIN_LEFT - MARGIN_RIGHT;
    double ratio = static_cast<double>(timestamp - m_viewStart) / m_viewDuration;
    
    return MARGIN_LEFT + static_cast<int>(ratio * timelineWidth);
}

QString TimelineWidget::formatTime(int64_t timestamp) const {
    int64_t totalSeconds = timestamp / 1000;
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds / 60) % 60;
    int seconds = totalSeconds % 60;
    
    if (hours > 0) {
        return QString("%1:%2:%3")
            .arg(hours)
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
    } else {
        return QString("%1:%2")
            .arg(minutes)
            .arg(seconds, 2, 10, QChar('0'));
    }
}

void TimelineWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (event->modifiers() & Qt::ShiftModifier) {
            m_dragging = true;
            m_dragStartX = event->pos().x();
            m_dragStartViewStart = m_viewStart;
            setCursor(Qt::ClosedHandCursor);
        } else {
            int64_t timestamp = positionToTimestamp(event->pos().x());
            timestamp = qBound(0LL, timestamp, m_duration);
            emit positionClicked(timestamp);
        }
    }
    
    QWidget::mousePressEvent(event);
}

void TimelineWidget::mouseMoveEvent(QMouseEvent* event) {
    m_hoverX = event->pos().x();
    
    if (m_dragging) {
        int dx = event->pos().x() - m_dragStartX;
        int timelineWidth = width() - MARGIN_LEFT - MARGIN_RIGHT;
        int64_t dt = static_cast<int64_t>(-dx * m_viewDuration / timelineWidth);
        
        m_viewStart = qBound(0LL, m_dragStartViewStart + dt, m_duration - m_viewDuration);
    }
    
    update();
    QWidget::mouseMoveEvent(event);
}

void TimelineWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (m_dragging) {
        m_dragging = false;
        setCursor(Qt::ArrowCursor);
    }
    
    QWidget::mouseReleaseEvent(event);
}

void TimelineWidget::wheelEvent(QWheelEvent* event) {
    int delta = event->angleDelta().y();
    
    if (delta > 0) {
        zoomIn();
    } else if (delta < 0) {
        zoomOut();
    }
    
    QWidget::wheelEvent(event);
}

void TimelineWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    update();
}

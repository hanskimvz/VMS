#include "video_widget.h"
#include "stream_receiver.h"

#include <QPainter>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QDebug>
#include <QKeyEvent>

VideoWidget::VideoWidget(QWidget* parent)
    : QWidget(parent)
    , m_updateTimer(new QTimer(this))
{
    setMinimumSize(80, 60);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAutoFillBackground(true);
    
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::black);
    setPalette(pal);
    
    connect(m_updateTimer, &QTimer::timeout, this, &VideoWidget::updateDisplay);
    m_updateTimer->start(UPDATE_INTERVAL_MS);
}

VideoWidget::~VideoWidget() {
    removeStreamReceiver();
}

void VideoWidget::setStreamReceiver(StreamReceiver* receiver) {
    removeStreamReceiver();
    
    m_receiver = receiver;
    if (m_receiver) {
        connect(m_receiver.data(), &StreamReceiver::frameReady,
                this, &VideoWidget::onFrameReady, Qt::QueuedConnection);
        connect(m_receiver.data(), &StreamReceiver::statsUpdated,
                this, &VideoWidget::onStatsUpdated, Qt::QueuedConnection);
    }
}

void VideoWidget::removeStreamReceiver() {
    if (m_receiver) {
        disconnect(m_receiver.data(), nullptr, this, nullptr);
    }
    m_receiver = nullptr;
    
    QMutexLocker locker(&m_frameMutex);
    m_currentFrame = QImage();
    m_scaledFrame = QImage();
    m_frameReady = false;
    
    m_currentFps = 0;
    m_avgDecodeTime = 0;
    m_decodeErrors = 0;
    m_framesReceived = 0;
    m_streamWidth = 0;
    m_streamHeight = 0;
    m_resolutionUpdated = false;
    
    update();
}

void VideoWidget::setShowStats(bool show) {
    m_showStats = show;
    
    if (show && m_receiver && !m_resolutionUpdated) {
        m_streamWidth = m_receiver->width();
        m_streamHeight = m_receiver->height();
        m_resolutionUpdated = true;
    }
    
    update();
}

void VideoWidget::onStatsUpdated(const StreamStats& stats) {
    m_currentFps = stats.currentFps;
    m_avgDecodeTime = stats.avgDecodeTimeMs;
    m_decodeErrors = stats.decodeErrors;
    m_framesReceived = stats.framesReceived;
    
    if (!m_resolutionUpdated && m_receiver) {
        m_streamWidth = m_receiver->width();
        m_streamHeight = m_receiver->height();
        if (m_streamWidth > 0 && m_streamHeight > 0) {
            m_resolutionUpdated = true;
        }
    }
    
    if (m_showStats) {
        update();
    }
}

void VideoWidget::setCameraName(const QString& name) {
    m_cameraName = name;
    update();
}

void VideoWidget::setSelected(bool selected) {
    m_selected = selected;
    update();
}

void VideoWidget::onFrameReady(const VideoFrame& frame) {
    QMutexLocker locker(&m_frameMutex);
    m_currentFrame = frame.image;
    m_frameReady = true;
}

void VideoWidget::displayFrame(const QImage& image) {
    QMutexLocker locker(&m_frameMutex);
    m_currentFrame = image;
    m_frameReady = true;
}

void VideoWidget::updateDisplay() {
    QMutexLocker locker(&m_frameMutex);
    
    if (m_currentFrame.isNull()) {
        return;
    }
    
    QSize targetSize = size();
    if (targetSize.isEmpty()) {
        return;
    }
    
    QSize scaledSize = m_currentFrame.size();
    scaledSize.scale(targetSize, Qt::KeepAspectRatio);
    
    bool sizeChanged = m_scaledFrame.isNull() || 
                       m_scaledFrame.width() != scaledSize.width() ||
                       m_scaledFrame.height() != scaledSize.height();
    
    if (m_frameReady || sizeChanged) {
        m_scaledFrame = m_currentFrame.scaled(scaledSize, 
                                               Qt::KeepAspectRatio, 
                                               Qt::SmoothTransformation);
        m_frameReady = false;
        locker.unlock();
        update();
    }
}

void VideoWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    
    painter.fillRect(rect(), Qt::black);
    
    bool hasFrame = false;
    QImage frameToDraw;
    
    {
        QMutexLocker locker(&m_frameMutex);
        if (!m_scaledFrame.isNull()) {
            frameToDraw = m_scaledFrame;
            hasFrame = true;
        }
    }
    
    if (hasFrame) {
        int x = (width() - frameToDraw.width()) / 2;
        int y = (height() - frameToDraw.height()) / 2;
        painter.drawImage(x, y, frameToDraw);
    } else {
        drawNoSignal(painter);
    }
    
    drawCameraName(painter);
    
    if (m_showStats && m_receiver) {
        drawStats(painter);
    }
    
    if (m_selected) {
        painter.setPen(QPen(QColor(0, 122, 204), 3));
        painter.drawRect(rect().adjusted(1, 1, -2, -2));
    }
}

void VideoWidget::drawNoSignal(QPainter& painter) {
    painter.setPen(QColor(60, 60, 60));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
    
    painter.setPen(QColor(80, 80, 80));
    painter.setFont(QFont("Arial", 12));
    
    QString text = m_cameraName.isEmpty() ? tr("No Signal") : m_cameraName;
    
    QRect textRect = painter.fontMetrics().boundingRect(text);
    int x = (width() - textRect.width()) / 2;
    int y = (height() + textRect.height()) / 2;
    
    painter.drawText(x, y, text);
}

void VideoWidget::drawCameraName(QPainter& painter) {
    if (m_cameraName.isEmpty()) {
        return;
    }
    
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    
    QRect textRect = painter.fontMetrics().boundingRect(m_cameraName);
    int padding = 4;
    
    QRect bgRect(5, 5, textRect.width() + padding * 2, textRect.height() + padding * 2);
    painter.fillRect(bgRect, QColor(0, 0, 0, 150));
    
    painter.drawText(bgRect, Qt::AlignCenter, m_cameraName);
}

void VideoWidget::drawStats(QPainter& painter) {
    painter.setFont(QFont("Consolas", 9));
    
    QStringList lines;
    if (m_streamWidth > 0 && m_streamHeight > 0) {
        lines << QString("%1x%2").arg(m_streamWidth).arg(m_streamHeight);
    }
    lines << QString("FPS: %1").arg(m_currentFps, 0, 'f', 1);
    lines << QString("Decode: %1ms").arg(m_avgDecodeTime, 0, 'f', 1);
    lines << QString("Errors: %1").arg(m_decodeErrors);
    
    int lineHeight = painter.fontMetrics().height();
    int padding = 4;
    int maxWidth = 0;
    
    for (const QString& line : lines) {
        maxWidth = qMax(maxWidth, painter.fontMetrics().horizontalAdvance(line));
    }
    
    int boxHeight = lines.size() * lineHeight + padding * 2;
    int boxWidth = maxWidth + padding * 2;
    
    QRect bgRect(width() - boxWidth - 5, 5, boxWidth, boxHeight);
    painter.fillRect(bgRect, QColor(0, 0, 0, 180));
    
    painter.setPen(m_decodeErrors > 0 ? QColor(255, 200, 100) : Qt::green);
    
    int y = bgRect.top() + padding + lineHeight - painter.fontMetrics().descent();
    for (const QString& line : lines) {
        painter.drawText(bgRect.left() + padding, y, line);
        y += lineHeight;
    }
}

void VideoWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked(m_cameraId);
    }
    QWidget::mousePressEvent(event);
}

void VideoWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit doubleClicked(m_cameraId);
    }
    QWidget::mouseDoubleClickEvent(event);
}

void VideoWidget::contextMenuEvent(QContextMenuEvent* event) {
    emit contextMenuRequested(m_cameraId, event->globalPos());
}

void VideoWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    
    {
        QMutexLocker locker(&m_frameMutex);
        m_scaledFrame = QImage();
    }
    
    QMetaObject::invokeMethod(this, &VideoWidget::updateDisplay, Qt::QueuedConnection);
}

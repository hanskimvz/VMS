#include "video_grid.h"
#include "video_widget.h"
#include "stream_receiver.h"

#include <QMenu>
#include <QSizePolicy>
#include <cmath>

VideoGrid::VideoGrid(QWidget* parent)
    : QWidget(parent)
    , m_gridLayout(new QGridLayout(this))
{
    m_gridLayout->setSpacing(2);
    m_gridLayout->setContentsMargins(2, 2, 2, 2);
    
    setLayout(4);
}

VideoGrid::~VideoGrid() {
    removeAllStreams();
}

void VideoGrid::setLayout(int cellCount) {
    // 지원하는 레이아웃: 1, 4, 6 (1+5), 9, 16
    if (cellCount != 1 && cellCount != 4 && cellCount != 6 && 
        cellCount != 9 && cellCount != 16) {
        return;
    }
    
    m_cellCount = cellCount;
    createWidgets(cellCount);
    arrangeWidgets();
}

void VideoGrid::createWidgets(int count) {
    while (m_widgets.size() > count) {
        VideoWidget* widget = m_widgets.takeLast();
        m_gridLayout->removeWidget(widget);
        delete widget;
    }
    
    while (m_widgets.size() < count) {
        VideoWidget* widget = new VideoWidget(this);
        
        connect(widget, &VideoWidget::clicked,
                this, &VideoGrid::onWidgetClicked);
        connect(widget, &VideoWidget::doubleClicked,
                this, &VideoGrid::onWidgetDoubleClicked);
        connect(widget, &VideoWidget::contextMenuRequested,
                this, &VideoGrid::onContextMenuRequested);
        
        m_widgets.append(widget);
    }
    
    QMap<QString, int> newMapping;
    for (auto it = m_cameraToWidget.begin(); it != m_cameraToWidget.end(); ++it) {
        if (it.value() < count) {
            newMapping.insert(it.key(), it.value());
        } else {
            VideoWidget* oldWidget = m_widgets.value(it.value());
            if (oldWidget) {
                oldWidget->removeStreamReceiver();
            }
        }
    }
    m_cameraToWidget = newMapping;
}

void VideoGrid::arrangeWidgets() {
    // 모든 위젯 제거
    for (int i = 0; i < m_widgets.size(); ++i) {
        m_gridLayout->removeWidget(m_widgets[i]);
    }
    
    // 이전 stretch 설정 초기화 (최대 4x4)
    for (int i = 0; i < 4; ++i) {
        m_gridLayout->setRowStretch(i, 0);
        m_gridLayout->setColumnStretch(i, 0);
    }
    
    // 특수 레이아웃: 6 (1+5)
    // [M M S]
    // [M M S]
    // [S S S]
    if (m_cellCount == 6) {
        // 메인 화면 (widget 0): 2x2 크기
        m_widgets[0]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_gridLayout->addWidget(m_widgets[0], 0, 0, 2, 2);
        m_widgets[0]->show();
        
        // 오른쪽 위 (widget 1)
        m_widgets[1]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_gridLayout->addWidget(m_widgets[1], 0, 2, 1, 1);
        m_widgets[1]->show();
        
        // 오른쪽 아래 (widget 2)
        m_widgets[2]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_gridLayout->addWidget(m_widgets[2], 1, 2, 1, 1);
        m_widgets[2]->show();
        
        // 하단 3개 (widget 3, 4, 5)
        for (int i = 0; i < 3; ++i) {
            m_widgets[i + 3]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            m_gridLayout->addWidget(m_widgets[i + 3], 2, i, 1, 1);
            m_widgets[i + 3]->show();
        }
        
        // stretch 설정
        for (int i = 0; i < 3; ++i) {
            m_gridLayout->setRowStretch(i, 1);
            m_gridLayout->setColumnStretch(i, 1);
        }
        
        return;
    }
    
    // 일반 그리드 레이아웃 (1x1, 2x2, 3x3, 4x4)
    int gridSize = static_cast<int>(std::sqrt(m_cellCount));
    
    // 위젯 배치
    for (int i = 0; i < m_widgets.size(); ++i) {
        int row = i / gridSize;
        int col = i % gridSize;
        m_gridLayout->addWidget(m_widgets[i], row, col);
        m_widgets[i]->show();
        m_widgets[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
    
    // stretch 설정
    for (int i = 0; i < gridSize; ++i) {
        m_gridLayout->setRowStretch(i, 1);
        m_gridLayout->setColumnStretch(i, 1);
    }
}

void VideoGrid::addStream(const QString& cameraId, const QString& name, StreamReceiver* receiver) {
    if (m_cameraToWidget.contains(cameraId)) {
        int index = m_cameraToWidget[cameraId];
        m_widgets[index]->setStreamReceiver(receiver);
        m_widgets[index]->setCameraName(name);
        return;
    }
    
    int slot = findEmptySlot();
    if (slot < 0) {
        slot = 0;
        m_widgets[slot]->removeStreamReceiver();
        
        QString oldCamera = m_cameraToWidget.key(slot);
        if (!oldCamera.isEmpty()) {
            m_cameraToWidget.remove(oldCamera);
        }
    }
    
    m_widgets[slot]->setCameraId(cameraId);
    m_widgets[slot]->setCameraName(name);
    m_widgets[slot]->setStreamReceiver(receiver);
    m_cameraToWidget.insert(cameraId, slot);
}

void VideoGrid::removeStream(const QString& cameraId) {
    if (!m_cameraToWidget.contains(cameraId)) {
        return;
    }
    
    int index = m_cameraToWidget[cameraId];
    m_widgets[index]->removeStreamReceiver();
    m_widgets[index]->setCameraId("");
    m_widgets[index]->setCameraName("");
    m_cameraToWidget.remove(cameraId);
}

void VideoGrid::removeAllStreams() {
    for (VideoWidget* widget : m_widgets) {
        widget->removeStreamReceiver();
        widget->setCameraId("");
        widget->setCameraName("");
    }
    m_cameraToWidget.clear();
}

VideoWidget* VideoGrid::getWidget(int index) const {
    if (index >= 0 && index < m_widgets.size()) {
        return m_widgets[index];
    }
    return nullptr;
}

VideoWidget* VideoGrid::getWidgetByCameraId(const QString& cameraId) const {
    if (m_cameraToWidget.contains(cameraId)) {
        return m_widgets[m_cameraToWidget[cameraId]];
    }
    return nullptr;
}

int VideoGrid::findEmptySlot() const {
    for (int i = 0; i < m_widgets.size(); ++i) {
        if (!m_widgets[i]->isPlaying()) {
            return i;
        }
    }
    return -1;
}

int VideoGrid::getWidgetIndex(const QString& cameraId) const {
    return m_cameraToWidget.value(cameraId, -1);
}

void VideoGrid::clearSelection() {
    for (VideoWidget* widget : m_widgets) {
        widget->setSelected(false);
    }
    m_selectedIndex = -1;
}

void VideoGrid::onWidgetClicked(const QString& cameraId) {
    clearSelection();
    
    int index = getWidgetIndex(cameraId);
    if (index >= 0) {
        m_widgets[index]->setSelected(true);
        m_selectedIndex = index;
    }
    
    emit widgetClicked(index, cameraId);
}

void VideoGrid::onWidgetDoubleClicked(const QString& cameraId) {
    int index = getWidgetIndex(cameraId);
    emit widgetDoubleClicked(index, cameraId);
}

void VideoGrid::onContextMenuRequested(const QString& cameraId, const QPoint& pos) {
    int index = getWidgetIndex(cameraId);
    emit contextMenuRequested(index, cameraId, pos);
}

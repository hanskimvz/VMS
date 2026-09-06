#include "video_grid.h"
#include "video_widget.h"
#include "stream_receiver.h"

#include <QMenu>
#include <QSizePolicy>
#include <cmath>
#include <algorithm>

VideoGrid::VideoGrid(QWidget* parent)
    : QWidget(parent)
    , m_gridLayout(new QGridLayout(this))
{
    m_gridLayout->setSpacing(2);
    m_gridLayout->setContentsMargins(2, 2, 2, 2);
    
    initLayout(4);
}

VideoGrid::~VideoGrid() {
    removeAllStreams();
}

void VideoGrid::initLayout(int cellCount) {
    m_cellCount = cellCount;
    createWidgets(cellCount);
    arrangeWidgets();
}

void VideoGrid::setLayout(int cellCount) {
    // 지원하는 레이아웃: 1, 4, 8 (1+7), 9, 16
    if (cellCount != 1 && cellCount != 4 && cellCount != 8 && 
        cellCount != 9 && cellCount != 16) {
        return;
    }
    
    if (m_cellCount == cellCount && !m_isMaximized) {
        return;
    }
    
    m_isMaximized = false;
    m_maximizedWidgetIndex = -1;
    m_cellCount = cellCount;
    m_previousLayout = cellCount;
    
    createWidgets(cellCount);
    arrangeWidgets();
    assignStreamsToWidgets();
    
    emit layoutChanged(cellCount);
}

void VideoGrid::createWidgets(int count) {
    for (VideoWidget* widget : m_widgets) {
        m_gridLayout->removeWidget(widget);
        widget->removeStreamReceiver();
        widget->hide();
    }
    
    while (m_widgets.size() > count) {
        VideoWidget* widget = m_widgets.takeLast();
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
}

void VideoGrid::assignStreamsToWidgets() {
    for (VideoWidget* widget : m_widgets) {
        widget->removeStreamReceiver();
        widget->setCameraId("");
        widget->setCameraName("");
    }
    
    QList<StreamInfo> sortedStreams = m_streams.values();
    std::sort(sortedStreams.begin(), sortedStreams.end(), 
              [](const StreamInfo& a, const StreamInfo& b) {
                  return a.slotIndex < b.slotIndex;
              });
    
    int widgetIndex = 0;
    for (const StreamInfo& stream : sortedStreams) {
        if (widgetIndex >= m_widgets.size()) {
            break;
        }
        
        VideoWidget* widget = m_widgets[widgetIndex];
        widget->setCameraId(stream.cameraId);
        widget->setCameraName(stream.name);
        widget->setStreamReceiver(stream.receiver.data());
        
        m_streams[stream.cameraId].slotIndex = widgetIndex;
        widgetIndex++;
    }
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
    
    // 특수 레이아웃: 8 (1+7)
    // [M M M S]
    // [M M M S]
    // [M M M S]
    // [S S S S]
    if (m_cellCount == 8) {
        // 메인 화면 (widget 0): 3x3 크기
        m_widgets[0]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_gridLayout->addWidget(m_widgets[0], 0, 0, 3, 3);
        m_widgets[0]->show();
        
        // 오른쪽 3개 (widget 1, 2, 3)
        for (int i = 0; i < 3; ++i) {
            m_widgets[i + 1]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            m_gridLayout->addWidget(m_widgets[i + 1], i, 3, 1, 1);
            m_widgets[i + 1]->show();
        }
        
        // 하단 4개 (widget 4, 5, 6, 7)
        for (int i = 0; i < 4; ++i) {
            m_widgets[i + 4]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            m_gridLayout->addWidget(m_widgets[i + 4], 3, i, 1, 1);
            m_widgets[i + 4]->show();
        }
        
        // stretch 설정
        for (int i = 0; i < 4; ++i) {
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
    if (m_streams.contains(cameraId)) {
        StreamInfo& stream = m_streams[cameraId];
        stream.name = name;
        stream.receiver = receiver;
        
        if (stream.slotIndex >= 0 && stream.slotIndex < m_widgets.size()) {
            m_widgets[stream.slotIndex]->setStreamReceiver(receiver);
            m_widgets[stream.slotIndex]->setCameraName(name);
        }
        return;
    }
    
    int slot = findEmptySlot();
    if (slot < 0) {
        // 빈 셀이 없으면 하나를 빼앗는다. 빼앗긴 스트림의 매핑을 남겨 두면 두 스트림이
        // 같은 슬롯을 가리키게 되므로 목록에서도 제거한다.
        slot = m_streams.size() % m_widgets.size();
        for (auto it = m_streams.begin(); it != m_streams.end(); ++it) {
            if (it->slotIndex == slot) {
                m_streams.erase(it);
                break;
            }
        }
    }
    
    StreamInfo stream;
    stream.cameraId = cameraId;
    stream.name = name;
    stream.receiver = receiver;
    stream.slotIndex = slot;
    m_streams.insert(cameraId, stream);
    
    if (slot < m_widgets.size()) {
        m_widgets[slot]->setCameraId(cameraId);
        m_widgets[slot]->setCameraName(name);
        m_widgets[slot]->setStreamReceiver(receiver);
    }
}

void VideoGrid::removeStream(const QString& cameraId) {
    if (!m_streams.contains(cameraId)) {
        return;
    }
    
    StreamInfo stream = m_streams.take(cameraId);
    
    if (stream.slotIndex >= 0 && stream.slotIndex < m_widgets.size()) {
        m_widgets[stream.slotIndex]->removeStreamReceiver();
        m_widgets[stream.slotIndex]->setCameraId("");
        m_widgets[stream.slotIndex]->setCameraName("");
    }
    
    assignStreamsToWidgets();
}

void VideoGrid::removeAllStreams() {
    for (VideoWidget* widget : m_widgets) {
        widget->removeStreamReceiver();
        widget->setCameraId("");
        widget->setCameraName("");
    }
    m_streams.clear();
}

VideoWidget* VideoGrid::getWidget(int index) const {
    if (index >= 0 && index < m_widgets.size()) {
        return m_widgets[index];
    }
    return nullptr;
}

VideoWidget* VideoGrid::getWidgetByCameraId(const QString& cameraId) const {
    if (m_streams.contains(cameraId)) {
        int index = m_streams[cameraId].slotIndex;
        if (index >= 0 && index < m_widgets.size()) {
            return m_widgets[index];
        }
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
    if (m_streams.contains(cameraId)) {
        return m_streams[cameraId].slotIndex;
    }
    return -1;
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
    int index = -1;
    
    for (int i = 0; i < m_widgets.size(); ++i) {
        if (m_widgets[i]->cameraId() == cameraId) {
            index = i;
            break;
        }
    }
    
    if (index < 0) {
        for (int i = 0; i < m_widgets.size(); ++i) {
            if (m_widgets[i]->geometry().contains(m_widgets[i]->mapFromGlobal(QCursor::pos()))) {
                index = i;
                break;
            }
        }
    }
    
    if (index >= 0) {
        toggleMaximize(index);
    }
    
    emit widgetDoubleClicked(index, cameraId);
}

void VideoGrid::toggleMaximize(int widgetIndex) {
    if (widgetIndex < 0 || widgetIndex >= m_widgets.size()) {
        return;
    }
    
    if (m_isMaximized) {
        m_isMaximized = false;
        m_maximizedWidgetIndex = -1;
        
        createWidgets(m_previousLayout);
        arrangeWidgets();
        assignStreamsToWidgets();
        
        emit layoutChanged(m_previousLayout);
    } else {
        if (m_cellCount > 1) {
            m_previousLayout = m_cellCount;
        }
        
        m_isMaximized = true;
        m_maximizedWidgetIndex = widgetIndex;
        
        arrangeWidgetsMaximized(widgetIndex);
        
        emit layoutChanged(1);
    }
}

void VideoGrid::arrangeWidgetsMaximized(int focusIndex) {
    for (int i = 0; i < m_widgets.size(); ++i) {
        m_gridLayout->removeWidget(m_widgets[i]);
        m_widgets[i]->hide();
    }
    
    for (int i = 0; i < 4; ++i) {
        m_gridLayout->setRowStretch(i, 0);
        m_gridLayout->setColumnStretch(i, 0);
    }
    
    if (focusIndex >= 0 && focusIndex < m_widgets.size()) {
        m_widgets[focusIndex]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_gridLayout->addWidget(m_widgets[focusIndex], 0, 0, 1, 1);
        m_widgets[focusIndex]->show();
        
        m_gridLayout->setRowStretch(0, 1);
        m_gridLayout->setColumnStretch(0, 1);
    }
}

void VideoGrid::onContextMenuRequested(const QString& cameraId, const QPoint& pos) {
    int index = getWidgetIndex(cameraId);
    emit contextMenuRequested(index, cameraId, pos);
}

void VideoGrid::toggleStats() {
    m_showStats = !m_showStats;
    for (VideoWidget* widget : m_widgets) {
        widget->setShowStats(m_showStats);
    }
}

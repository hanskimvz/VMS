#include "camera_tree.h"
#include "camera_manager.h"
#include "camera.h"

#include <QHeaderView>
#include <QMenu>
#include <QContextMenuEvent>
#include <QMessageBox>
#include <QPainter>
#include <QThreadPool>
#include <QRunnable>
#include <QTcpSocket>

CameraTree::CameraTree(QWidget* parent)
    : QTreeWidget(parent)
{
    setupUi();
    
    connect(this, &QTreeWidget::itemSelectionChanged,
            this, &CameraTree::onItemSelectionChanged);
    connect(this, &QTreeWidget::itemDoubleClicked,
            this, &CameraTree::onItemDoubleClicked);
    
    m_statusCheckTimer = new QTimer(this);
    connect(m_statusCheckTimer, &QTimer::timeout, this, &CameraTree::checkCameraStatus);
    m_statusCheckTimer->start(30000);
}

CameraTree::~CameraTree() {
    if (m_statusCheckTimer) {
        m_statusCheckTimer->stop();
    }
}

void CameraTree::setupUi() {
    setHeaderHidden(true);
    setRootIsDecorated(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragOnly);
    
    setStyleSheet(
        "QTreeWidget { "
        "   background-color: #252526; "
        "   border: none; "
        "   color: #cccccc; "
        "   font-size: 12px; "
        "} "
        "QTreeWidget::item { "
        "   padding: 4px 2px; "
        "   border: none; "
        "} "
        "QTreeWidget::item:hover { "
        "   background-color: #2a2d2e; "
        "} "
        "QTreeWidget::item:selected { "
        "   background-color: #094771; "
        "   color: white; "
        "} "
        "QTreeWidget::branch:has-children:!has-siblings:closed, "
        "QTreeWidget::branch:closed:has-children:has-siblings { "
        "   border-image: none; "
        "   image: url(none); "
        "} "
        "QTreeWidget::branch:open:has-children:!has-siblings, "
        "QTreeWidget::branch:open:has-children:has-siblings { "
        "   border-image: none; "
        "   image: url(none); "
        "} "
    );
    
    m_defaultGroupItem = new QTreeWidgetItem(this);
    m_defaultGroupItem->setText(0, tr("Default"));
    m_defaultGroupItem->setExpanded(true);
    
    QFont groupFont = m_defaultGroupItem->font(0);
    groupFont.setBold(true);
    m_defaultGroupItem->setFont(0, groupFont);
    m_defaultGroupItem->setForeground(0, QColor("#e0e0e0"));
}

void CameraTree::setCameraManager(CameraManager* manager) {
    m_cameraManager = manager;
    
    if (m_cameraManager) {
        connect(m_cameraManager, &CameraManager::cameraStatusChanged,
                this, &CameraTree::onCameraStatusChanged);
        connect(m_cameraManager, &CameraManager::cameraAdded,
                this, &CameraTree::refreshCameras);
        connect(m_cameraManager, &CameraManager::cameraRemoved,
                this, &CameraTree::refreshCameras);
        connect(m_cameraManager, &CameraManager::cameraUpdated,
                this, &CameraTree::refreshCameras);
    }
    
    refreshCameras();
    
    QTimer::singleShot(500, this, &CameraTree::checkCameraStatus);
}

void CameraTree::refreshCameras() {
    while (m_defaultGroupItem->childCount() > 0) {
        delete m_defaultGroupItem->takeChild(0);
    }
    m_cameraItems.clear();
    
    if (!m_cameraManager) {
        return;
    }
    
    QList<CameraInfo> cameras = m_cameraManager->getAllCameras();
    
    for (const CameraInfo& camera : cameras) {
        addCameraItem(camera);
    }
    
    m_defaultGroupItem->setText(0, tr("Default (%1)").arg(m_defaultGroupItem->childCount()));
}

QIcon CameraTree::createStatusIcon(bool isOnline) const {
    int size = 12;
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QColor color = isOnline ? QColor(0, 200, 83) : QColor(244, 67, 54);
    
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawEllipse(1, 1, size - 2, size - 2);
    
    if (isOnline) {
        QColor highlight(150, 255, 180, 100);
        painter.setBrush(highlight);
        painter.drawEllipse(3, 2, 4, 4);
    }
    
    return QIcon(pixmap);
}

void CameraTree::addCameraItem(const CameraInfo& camera) {
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, camera.name);
    item->setData(0, Qt::UserRole, camera.id);
    item->setToolTip(0, QString("%1\nIP: %2\nStatus: %3")
        .arg(camera.name)
        .arg(camera.ip)
        .arg(camera.status == CameraStatus::Online ? tr("Online") :
             camera.status == CameraStatus::Offline ? tr("Offline") : tr("Checking...")));
    
    // 아이콘은 "도달 가능"(30초 TCP 핑) 여부다. 스트리밍 여부(CameraStatus)와는 별개이므로
    // 이미 핑으로 알아낸 값이 있으면 그것을 유지한다.
    bool isOnline = m_cameraStatusCache.value(camera.id, camera.status == CameraStatus::Online);
    m_cameraStatusCache[camera.id] = isOnline;
    item->setIcon(0, createStatusIcon(isOnline));
    
    m_defaultGroupItem->addChild(item);
    m_cameraItems.insert(camera.id, item);
}

class CameraStatusChecker : public QRunnable {
public:
    CameraStatusChecker(CameraTree* tree, const QString& cameraId, const QString& ip, int port)
        : m_tree(tree), m_cameraId(cameraId), m_ip(ip), m_port(port) {}
    
    void run() override {
        bool isOnline = false;
        
        QTcpSocket socket;
        socket.connectToHost(m_ip, m_port);
        isOnline = socket.waitForConnected(2000);
        socket.close();
        
        QMetaObject::invokeMethod(m_tree, [tree = m_tree, cameraId = m_cameraId, isOnline]() {
            tree->updateCameraStatus(cameraId, isOnline);
        }, Qt::QueuedConnection);
    }
    
private:
    CameraTree* m_tree;
    QString m_cameraId;
    QString m_ip;
    int m_port;
};

void CameraTree::checkCameraStatus() {
    if (!m_cameraManager) {
        return;
    }
    
    QList<CameraInfo> cameras = m_cameraManager->getAllCameras();
    
    for (const CameraInfo& camera : cameras) {
        CameraStatusChecker* checker = new CameraStatusChecker(this, camera.id, camera.ip, camera.port);
        QThreadPool::globalInstance()->start(checker);
    }
}

void CameraTree::updateCameraStatus(const QString& cameraId, bool isOnline) {
    if (!m_cameraItems.contains(cameraId)) {
        return;
    }
    
    if (m_cameraStatusCache.contains(cameraId) && m_cameraStatusCache[cameraId] == isOnline) {
        return;
    }
    
    m_cameraStatusCache[cameraId] = isOnline;
    
    QTreeWidgetItem* item = m_cameraItems[cameraId];
    item->setIcon(0, createStatusIcon(isOnline));
    
    if (m_cameraManager) {
        CameraInfo camera = m_cameraManager->getCamera(cameraId);
        item->setToolTip(0, QString("%1\nIP: %2\nStatus: %3")
            .arg(camera.name)
            .arg(camera.ip)
            .arg(isOnline ? tr("Online") : tr("Offline")));
    }
}

void CameraTree::onItemSelectionChanged() {
    QString cameraId = getSelectedCameraId();
    if (!cameraId.isEmpty()) {
        emit cameraSelected(cameraId);
    }
}

void CameraTree::onItemDoubleClicked(QTreeWidgetItem* item, int column) {
    Q_UNUSED(column);
    
    QString cameraId = item->data(0, Qt::UserRole).toString();
    if (!cameraId.isEmpty()) {
        emit cameraDoubleClicked(cameraId);
    }
}

void CameraTree::contextMenuEvent(QContextMenuEvent* event) {
    QTreeWidgetItem* item = itemAt(event->pos());
    
    if (!item) {
        return;
    }
    
    QString cameraId = item->data(0, Qt::UserRole).toString();
    if (cameraId.isEmpty()) {
        return;
    }
    
    QMenu menu(this);
    menu.setStyleSheet(
        "QMenu { background-color: #2d2d30; border: 1px solid #3f3f46; color: #cccccc; } "
        "QMenu::item:selected { background-color: #094771; } "
        "QMenu::separator { background-color: #3f3f46; height: 1px; margin: 4px 10px; }"
    );
    
    // 메뉴는 실제 스트리밍 여부로 결정한다. 핑 결과(도달 가능)와 혼동하면
    // 재생 중이 아닌 카메라에 "Stop Stream" 이 뜬다.
    bool isStreaming = m_cameraManager && m_cameraManager->getStreamReceiver(cameraId) != nullptr;

    if (isStreaming) {
        menu.addAction(tr("Stop Stream"), this, &CameraTree::onStopStream);
    } else {
        menu.addAction(tr("Start Stream"), this, &CameraTree::onStartStream);
    }
    
    menu.addSeparator();
    menu.addAction(tr("Edit..."), this, &CameraTree::onEditCamera);
    menu.addAction(tr("Delete"), this, &CameraTree::onDeleteCamera);
    
    menu.exec(event->globalPos());
}

QString CameraTree::getSelectedCameraId() const {
    QList<QTreeWidgetItem*> selected = selectedItems();
    if (selected.isEmpty()) {
        return QString();
    }
    
    return selected.first()->data(0, Qt::UserRole).toString();
}

void CameraTree::onStartStream() {
    QString cameraId = getSelectedCameraId();
    if (!cameraId.isEmpty()) {
        // CameraManager 를 직접 부르면 그리드에 배치되지 않아 화면 없이 디코드만 한다.
        emit streamStartRequested(cameraId);
    }
}

void CameraTree::onCameraStatusChanged(const QString& cameraId, CameraStatus status) {
    // 스트림이 열렸다는 것은 도달 가능하다는 뜻이므로 핑을 기다리지 않고 바로 반영한다.
    // 스트림이 닫힌 것은 도달 불가를 뜻하지 않으므로 그대로 둔다.
    if (status == CameraStatus::Online) {
        updateCameraStatus(cameraId, true);
    }
}

void CameraTree::onStopStream() {
    QString cameraId = getSelectedCameraId();
    if (!cameraId.isEmpty() && m_cameraManager) {
        m_cameraManager->stopStream(cameraId);
    }
}

void CameraTree::onEditCamera() {
    QString cameraId = getSelectedCameraId();
    if (!cameraId.isEmpty()) {
        emit cameraEditRequested(cameraId);
    }
}

void CameraTree::onDeleteCamera() {
    QString cameraId = getSelectedCameraId();
    if (cameraId.isEmpty() || !m_cameraManager) {
        return;
    }
    
    CameraInfo camera = m_cameraManager->getCamera(cameraId);
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("Delete Camera"),
        tr("Are you sure you want to delete camera '%1'?").arg(camera.name),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        m_cameraManager->removeCamera(cameraId);
    }
}

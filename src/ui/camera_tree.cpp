#include "camera_tree.h"
#include "camera_manager.h"
#include "camera.h"

#include <QHeaderView>
#include <QMenu>
#include <QContextMenuEvent>
#include <QMessageBox>

CameraTree::CameraTree(QWidget* parent)
    : QTreeWidget(parent)
{
    setupUi();
    
    connect(this, &QTreeWidget::itemSelectionChanged,
            this, &CameraTree::onItemSelectionChanged);
    connect(this, &QTreeWidget::itemDoubleClicked,
            this, &CameraTree::onItemDoubleClicked);
}

CameraTree::~CameraTree() {
}

void CameraTree::setupUi() {
    setHeaderHidden(true);
    setRootIsDecorated(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragOnly);
    
    m_onlineCamerasItem = new QTreeWidgetItem(this);
    m_onlineCamerasItem->setText(0, tr("Online Cameras"));
    m_onlineCamerasItem->setExpanded(true);
    
    m_offlineCamerasItem = new QTreeWidgetItem(this);
    m_offlineCamerasItem->setText(0, tr("Offline Cameras"));
    m_offlineCamerasItem->setExpanded(true);
}

void CameraTree::setCameraManager(CameraManager* manager) {
    m_cameraManager = manager;
    
    if (m_cameraManager) {
        connect(m_cameraManager, &CameraManager::cameraStatusChanged,
                this, &CameraTree::refreshCameras);
    }
    
    refreshCameras();
}

void CameraTree::refreshCameras() {
    while (m_onlineCamerasItem->childCount() > 0) {
        delete m_onlineCamerasItem->takeChild(0);
    }
    while (m_offlineCamerasItem->childCount() > 0) {
        delete m_offlineCamerasItem->takeChild(0);
    }
    m_cameraItems.clear();
    
    if (!m_cameraManager) {
        return;
    }
    
    QList<CameraInfo> cameras = m_cameraManager->getAllCameras();
    
    for (const CameraInfo& camera : cameras) {
        addCameraItem(camera);
    }
    
    m_onlineCamerasItem->setText(0, tr("Online Cameras (%1)").arg(m_onlineCamerasItem->childCount()));
    m_offlineCamerasItem->setText(0, tr("Offline Cameras (%1)").arg(m_offlineCamerasItem->childCount()));
}

void CameraTree::addCameraItem(const CameraInfo& camera) {
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, camera.name);
    item->setData(0, Qt::UserRole, camera.id);
    item->setToolTip(0, QString("%1\nIP: %2").arg(camera.name).arg(camera.ip));
    
    if (camera.status == CameraStatus::Online) {
        m_onlineCamerasItem->addChild(item);
    } else {
        m_offlineCamerasItem->addChild(item);
    }
    
    m_cameraItems.insert(camera.id, item);
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
    
    CameraInfo camera = m_cameraManager->getCamera(cameraId);
    
    if (camera.status == CameraStatus::Online) {
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
    if (!cameraId.isEmpty() && m_cameraManager) {
        m_cameraManager->startStream(cameraId);
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
    if (cameraId.isEmpty()) {
        return;
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

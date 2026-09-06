#include "device_manage_widget.h"
#include "camera_manager.h"
#include "onvif_client.h"
#include "device_discovery.h"
#include "add_camera_dialog.h"
#include "network_settings_dialog.h"
#include "camera.h"
#include "local_interfaces.h"

#include <QTimer>
#include <QShowEvent>

DeviceManageWidget::DeviceManageWidget(QWidget* parent)
    : QWidget(parent)
{
    m_deviceDiscovery = new DeviceDiscovery(this);
    connect(m_deviceDiscovery, &DeviceDiscovery::deviceDiscovered,
            this, &DeviceManageWidget::onOtherDeviceDiscovered);
    connect(m_deviceDiscovery, &DeviceDiscovery::discoveryFinished,
            this, &DeviceManageWidget::onOtherDiscoveryFinished);
    
    setupUi();
    refreshInterfaceList();
}

DeviceManageWidget::~DeviceManageWidget() {
    if (m_deviceDiscovery) {
        m_deviceDiscovery->stopDiscovery();
    }
}

void DeviceManageWidget::setCameraManager(CameraManager* manager) {
    m_cameraManager = manager;
    if (m_cameraManager) {
        connect(m_cameraManager, &CameraManager::cameraAdded,
                this, &DeviceManageWidget::refreshAddedDevices);
        connect(m_cameraManager, &CameraManager::cameraRemoved,
                this, &DeviceManageWidget::refreshAddedDevices);
        connect(m_cameraManager, &CameraManager::cameraUpdated,
                this, &DeviceManageWidget::refreshAddedDevices);
        connect(m_cameraManager, &CameraManager::cameraStatusChanged,
                this, &DeviceManageWidget::refreshAddedDevices);
        refreshAddedDevices();
    }
}

void DeviceManageWidget::setOnvifClient(OnvifClient* client) {
    m_onvifClient = client;
    if (m_onvifClient) {
        connect(m_onvifClient, &OnvifClient::deviceDiscovered,
                this, &DeviceManageWidget::onOnvifDeviceDiscovered);
        connect(m_onvifClient, &OnvifClient::discoveryFinished,
                this, &DeviceManageWidget::onOnvifDiscoveryFinished);
    }
}

void DeviceManageWidget::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);
    
    setupSearchedDeviceGroup();
    mainLayout->addWidget(m_searchedGroup, 1);
    
    setupAddedDeviceGroup();
    mainLayout->addWidget(m_addedGroup, 1);
}

void DeviceManageWidget::setupSearchedDeviceGroup() {
    m_searchedGroup = new QGroupBox(tr("Searched device"), this);
    m_searchedGroup->setStyleSheet(
        "QGroupBox { "
        "   font-weight: bold; "
        "   color: #4da6ff; "
        "   border: 1px solid #3a3a3a; "
        "   border-radius: 5px; "
        "   margin-top: 10px; "
        "   padding-top: 10px; "
        "} "
        "QGroupBox::title { "
        "   subcontrol-origin: margin; "
        "   left: 10px; "
        "   padding: 0 5px; "
        "}"
    );
    
    QVBoxLayout* layout = new QVBoxLayout(m_searchedGroup);
    layout->setContentsMargins(10, 20, 10, 10);
    layout->setSpacing(10);
    
    // Buttons
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);
    
    auto createButton = [](const QString& text) -> QPushButton* {
        QPushButton* btn = new QPushButton(text);
        btn->setFixedHeight(30);
        btn->setMinimumWidth(100);
        btn->setStyleSheet(
            "QPushButton { "
            "   background-color: #e0e0e0; "
            "   color: #333; "
            "   border: 1px solid #ccc; "
            "   border-radius: 3px; "
            "   padding: 5px 15px; "
            "} "
            "QPushButton:hover { background-color: #d0d0d0; } "
            "QPushButton:pressed { background-color: #c0c0c0; }"
        );
        return btn;
    };
    
    m_startSearchBtn = createButton(tr("Start Search"));
    m_addDeviceBtn = createButton(tr("Add Device"));
    m_modifyIPBtn = createButton(tr("Modify IP"));
    m_manualAddBtn = createButton(tr("Manual Add"));
    
    btnLayout->addWidget(m_startSearchBtn);
    btnLayout->addWidget(m_addDeviceBtn);
    btnLayout->addWidget(m_modifyIPBtn);
    btnLayout->addWidget(m_manualAddBtn);

    // 프로브를 내보낼 NIC. VPN/가상 어댑터가 기본 경로를 잡고 있는 PC 에서는 이 선택이 검색 성패를 가른다.
    btnLayout->addSpacing(20);
    QLabel* ifaceLabel = new QLabel(tr("Interface:"), m_searchedGroup);
    ifaceLabel->setStyleSheet("color: #cccccc; font-weight: normal;");
    btnLayout->addWidget(ifaceLabel);
    m_interfaceCombo = new QComboBox(m_searchedGroup);
    m_interfaceCombo->setMinimumWidth(240);
    m_interfaceCombo->setStyleSheet("font-weight: normal;");
    m_interfaceCombo->setToolTip(tr("Network interface used to send discovery probes.\n"
                                    "Choose the one connected to the camera LAN, or All."));
    btnLayout->addWidget(m_interfaceCombo);
    btnLayout->addStretch();
    
    layout->addLayout(btnLayout);
    
    m_searchStatusLabel = new QLabel(tr("Press Start Search to discover cameras."), m_searchedGroup);
    m_searchStatusLabel->setStyleSheet("color: #888888; font-weight: normal;");
    layout->addWidget(m_searchStatusLabel);

    // Table
    m_searchedTable = new QTableWidget(this);
    m_searchedTable->setColumnCount(5);
    m_searchedTable->setHorizontalHeaderLabels({tr("IP"), tr("Name"), tr("Type"), tr("Model"), tr("Service URL")});
    m_searchedTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_searchedTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_searchedTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_searchedTable->setAlternatingRowColors(true);
    m_searchedTable->verticalHeader()->setVisible(false);
    m_searchedTable->horizontalHeader()->setStretchLastSection(true);
    m_searchedTable->setColumnWidth(0, 130);
    m_searchedTable->setColumnWidth(1, 200);
    m_searchedTable->setColumnWidth(2, 70);
    m_searchedTable->setColumnWidth(3, 150);
    applyTableStyle(m_searchedTable);
    
    layout->addWidget(m_searchedTable);
    
    // Connect signals
    connect(m_startSearchBtn, &QPushButton::clicked, this, &DeviceManageWidget::onStartSearch);
    connect(m_addDeviceBtn, &QPushButton::clicked, this, &DeviceManageWidget::onAddDevice);
    connect(m_modifyIPBtn, &QPushButton::clicked, this, &DeviceManageWidget::onModifyIP);
    connect(m_manualAddBtn, &QPushButton::clicked, this, &DeviceManageWidget::onManualAdd);
    connect(m_searchedTable, &QTableWidget::cellDoubleClicked, this, &DeviceManageWidget::onSearchedDeviceDoubleClicked);
}

void DeviceManageWidget::setupAddedDeviceGroup() {
    m_addedGroup = new QGroupBox(tr("Added device"), this);
    m_addedGroup->setStyleSheet(
        "QGroupBox { "
        "   font-weight: bold; "
        "   color: #4da6ff; "
        "   border: 1px solid #3a3a3a; "
        "   border-radius: 5px; "
        "   margin-top: 10px; "
        "   padding-top: 10px; "
        "} "
        "QGroupBox::title { "
        "   subcontrol-origin: margin; "
        "   left: 10px; "
        "   padding: 0 5px; "
        "}"
    );
    
    QVBoxLayout* layout = new QVBoxLayout(m_addedGroup);
    layout->setContentsMargins(10, 20, 10, 10);
    layout->setSpacing(10);
    
    // Buttons
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);
    
    auto createButton = [](const QString& text) -> QPushButton* {
        QPushButton* btn = new QPushButton(text);
        btn->setFixedHeight(30);
        btn->setMinimumWidth(100);
        btn->setStyleSheet(
            "QPushButton { "
            "   background-color: #e0e0e0; "
            "   color: #333; "
            "   border: 1px solid #ccc; "
            "   border-radius: 3px; "
            "   padding: 5px 15px; "
            "} "
            "QPushButton:hover { background-color: #d0d0d0; } "
            "QPushButton:pressed { background-color: #c0c0c0; }"
        );
        return btn;
    };
    
    m_deleteBtn = createButton(tr("Delete"));
    m_editBtn = createButton(tr("Edit"));
    
    btnLayout->addWidget(m_deleteBtn);
    btnLayout->addWidget(m_editBtn);
    btnLayout->addStretch();
    
    layout->addLayout(btnLayout);
    
    // Table
    m_addedTable = new QTableWidget(this);
    m_addedTable->setColumnCount(7);
    m_addedTable->setHorizontalHeaderLabels({
        tr("Name"), tr("IP"), tr("Model"), tr("Device SN"), 
        tr("Device Type"), tr("Connect State"), tr("File System Version")
    });
    m_addedTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_addedTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_addedTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_addedTable->setAlternatingRowColors(true);
    m_addedTable->verticalHeader()->setVisible(false);
    m_addedTable->horizontalHeader()->setStretchLastSection(true);
    m_addedTable->setColumnWidth(0, 120);
    m_addedTable->setColumnWidth(1, 120);
    m_addedTable->setColumnWidth(2, 150);
    m_addedTable->setColumnWidth(3, 120);
    m_addedTable->setColumnWidth(4, 100);
    m_addedTable->setColumnWidth(5, 100);
    applyTableStyle(m_addedTable);
    
    layout->addWidget(m_addedTable);
    
    // Connect signals
    connect(m_deleteBtn, &QPushButton::clicked, this, &DeviceManageWidget::onDeleteDevice);
    connect(m_editBtn, &QPushButton::clicked, this, &DeviceManageWidget::onEditDevice);
    connect(m_addedTable, &QTableWidget::cellDoubleClicked, this, &DeviceManageWidget::onAddedDeviceDoubleClicked);
}

void DeviceManageWidget::applyTableStyle(QTableWidget* table) {
    table->setStyleSheet(
        "QTableWidget { "
        "   background-color: #ffffff; "
        "   color: #333; "
        "   gridline-color: #ddd; "
        "   border: 1px solid #ccc; "
        "} "
        "QTableWidget::item { "
        "   padding: 5px; "
        "} "
        "QTableWidget::item:selected { "
        "   background-color: #cce5ff; "
        "   color: #333; "
        "} "
        "QHeaderView::section { "
        "   background-color: #f5f5f5; "
        "   color: #333; "
        "   padding: 8px; "
        "   border: none; "
        "   border-bottom: 1px solid #ddd; "
        "   border-right: 1px solid #ddd; "
        "   font-weight: normal; "
        "}"
        "QTableWidget::item:alternate { "
        "   background-color: #fafafa; "
        "}"
    );
}

void DeviceManageWidget::onStartSearch() {
    qDebug() << "DeviceManageWidget: Starting search...";
    m_searchedTable->setRowCount(0);
    updateSearchedGroupTitle();
    m_startSearchBtn->setText(tr("Searching..."));
    m_startSearchBtn->setEnabled(false);
    
    // Reset discovery state
    m_onvifDiscoveredIPs.clear();
    m_pendingUpnpDevices.clear();
    m_pendingMdnsDevices.clear();
    m_onvifDiscoveryFinished = false;
    m_otherDiscoveryFinished = false;

    const QList<QHostAddress> localAddresses = selectedLocalAddresses();
    QStringList interfaceLabels;
    for (const LocalInterface& li : resolveDiscoveryInterfaces(localAddresses)) {
        interfaceLabels << li.label();
    }
    if (interfaceLabels.isEmpty()) {
        m_searchStatusLabel->setText(tr("No usable IPv4 network interface. Check the network connection."));
    } else {
        m_searchStatusLabel->setText(tr("Searching on: %1").arg(interfaceLabels.join(", ")));
    }
    qInfo() << "DeviceManageWidget: search interfaces:" << interfaceLabels;
    
    // Start ONVIF WS-Discovery (highest priority)
    if (m_onvifClient) {
        qDebug() << "DeviceManageWidget: Starting ONVIF discovery...";
        m_onvifClient->discover(5000, localAddresses);
    } else {
        qDebug() << "DeviceManageWidget: WARNING - m_onvifClient is null!";
        m_onvifDiscoveryFinished = true;  // Mark as done if no client
    }
    
    // Start mDNS and UPnP/SSDP discovery
    if (m_deviceDiscovery) {
        qDebug() << "DeviceManageWidget: Starting mDNS/SSDP discovery...";
        m_deviceDiscovery->startDiscovery(5000, localAddresses);
    } else {
        qDebug() << "DeviceManageWidget: WARNING - m_deviceDiscovery is null!";
        m_otherDiscoveryFinished = true;  // Mark as done if no discovery
    }
    
    // If both are null, enable button immediately
    if (m_onvifDiscoveryFinished && m_otherDiscoveryFinished) {
        qDebug() << "DeviceManageWidget: No discovery modules available!";
        m_startSearchBtn->setText(tr("Start Search"));
        m_startSearchBtn->setEnabled(true);
    }
}

void DeviceManageWidget::onOnvifDeviceDiscovered(const OnvifDevice& device) {
    // ONVIF devices are added immediately (highest priority)
    m_onvifDiscoveredIPs.insert(device.address);
    addDeviceToSearchedTable(device);
}

void DeviceManageWidget::onOnvifDiscoveryFinished() {
    qDebug() << "DeviceManageWidget: ONVIF discovery finished";
    m_onvifDiscoveryFinished = true;
    
    // Now flush pending UPnP/mDNS devices that aren't already discovered via ONVIF
    flushPendingDevices();
    
    // Check if we should enable button
    if (m_otherDiscoveryFinished) {
        qDebug() << "DeviceManageWidget: Both discoveries finished, enabling button";
        m_startSearchBtn->setText(tr("Start Search"));
        m_startSearchBtn->setEnabled(true);
        m_searchStatusLabel->setText(tr("Search finished: %1 device(s) found.").arg(m_searchedTable->rowCount()));
    }
}

void DeviceManageWidget::onOtherDeviceDiscovered(const DiscoveredDevice& device) {
    qDebug() << "DeviceManageWidget: Other device discovered:" << device.ip << "type:" << device.discoveryType;
    
    // ONVIF devices from DeviceDiscovery should be treated same as OnvifClient devices
    if (device.discoveryType == "ONVIF") {
        // Convert to OnvifDevice and add directly
        if (!m_onvifDiscoveredIPs.contains(device.ip)) {
            m_onvifDiscoveredIPs.insert(device.ip);
            OnvifDevice onvifDevice;
            onvifDevice.address = device.ip;
            onvifDevice.name = device.name;
            onvifDevice.model = device.model;
            onvifDevice.manufacturer = device.manufacturer;
            onvifDevice.serviceUrl = device.serviceUrl;
            onvifDevice.xaddr = device.serviceUrl;
            addDeviceToSearchedTable(onvifDevice);
        }
        return;
    }
    
    // Queue UPnP/mDNS devices until ONVIF discovery completes
    if (device.discoveryType == "UPnP") {
        m_pendingUpnpDevices.append(device);
    } else {
        m_pendingMdnsDevices.append(device);
    }
    
    // If ONVIF already finished, add immediately (checking for duplicates)
    if (m_onvifDiscoveryFinished) {
        flushPendingDevices();
    }
}

void DeviceManageWidget::onOtherDiscoveryFinished() {
    qDebug() << "DeviceManageWidget: Other (mDNS/SSDP/WS-Discovery) discovery finished";
    m_otherDiscoveryFinished = true;
    flushPendingDevices();
    
    // Enable button when both discoveries are done
    if (m_onvifDiscoveryFinished && m_otherDiscoveryFinished) {
        qDebug() << "DeviceManageWidget: Both discoveries finished, enabling button";
        m_startSearchBtn->setText(tr("Start Search"));
        m_startSearchBtn->setEnabled(true);
        m_searchStatusLabel->setText(tr("Search finished: %1 device(s) found.").arg(m_searchedTable->rowCount()));
    } else {
        qDebug() << "DeviceManageWidget: Waiting for ONVIF discovery to finish";
    }
}

void DeviceManageWidget::flushPendingDevices() {
    // Add UPnP devices first (higher priority than mDNS)
    for (const auto& device : m_pendingUpnpDevices) {
        if (!m_onvifDiscoveredIPs.contains(device.ip)) {
            addDiscoveredDeviceToTable(device);
        }
    }
    m_pendingUpnpDevices.clear();
    
    // Then add mDNS devices
    for (const auto& device : m_pendingMdnsDevices) {
        if (!m_onvifDiscoveredIPs.contains(device.ip)) {
            addDiscoveredDeviceToTable(device);
        }
    }
    m_pendingMdnsDevices.clear();
}

bool DeviceManageWidget::isDeviceAlreadyAdded(const QString& ip) const {
    if (!m_cameraManager) return false;
    
    QList<CameraInfo> cameras = m_cameraManager->getAllCameras();
    for (const CameraInfo& cam : cameras) {
        if (cam.ip == ip) {
            return true;
        }
    }
    return false;
}

void DeviceManageWidget::addDeviceToSearchedTable(const OnvifDevice& device) {
    // Skip if already added to camera manager
    if (isDeviceAlreadyAdded(device.address)) {
        return;
    }
    
    // Check for duplicate IP in search table
    for (int i = 0; i < m_searchedTable->rowCount(); ++i) {
        if (m_searchedTable->item(i, 0)->text() == device.address) {
            return;  // Already exists
        }
    }
    
    int row = m_searchedTable->rowCount();
    m_searchedTable->insertRow(row);
    
    m_searchedTable->setItem(row, 0, new QTableWidgetItem(device.address));
    m_searchedTable->setItem(row, 1, new QTableWidgetItem(device.name));
    m_searchedTable->setItem(row, 2, new QTableWidgetItem("ONVIF"));
    m_searchedTable->setItem(row, 3, new QTableWidgetItem(device.model));
    m_searchedTable->setItem(row, 4, new QTableWidgetItem(device.serviceUrl));
    
    // Store service URL and type in first column's data
    m_searchedTable->item(row, 0)->setData(Qt::UserRole, device.serviceUrl);
    m_searchedTable->item(row, 0)->setData(Qt::UserRole + 1, "ONVIF");
    
    updateSearchedGroupTitle();
}

void DeviceManageWidget::addDiscoveredDeviceToTable(const DiscoveredDevice& device) {
    // Skip if already added to camera manager
    if (isDeviceAlreadyAdded(device.ip)) {
        return;
    }
    
    // Check for duplicate IP (including ONVIF devices)
    if (m_onvifDiscoveredIPs.contains(device.ip)) {
        return;  // Already discovered via ONVIF
    }
    
    for (int i = 0; i < m_searchedTable->rowCount(); ++i) {
        if (m_searchedTable->item(i, 0)->text() == device.ip) {
            return;  // Already exists
        }
    }
    
    int row = m_searchedTable->rowCount();
    m_searchedTable->insertRow(row);
    
    m_searchedTable->setItem(row, 0, new QTableWidgetItem(device.ip));
    m_searchedTable->setItem(row, 1, new QTableWidgetItem(device.name));
    m_searchedTable->setItem(row, 2, new QTableWidgetItem(device.discoveryType));
    m_searchedTable->setItem(row, 3, new QTableWidgetItem(device.model));
    m_searchedTable->setItem(row, 4, new QTableWidgetItem(device.serviceUrl));
    
    // Store service URL and type in first column's data
    m_searchedTable->item(row, 0)->setData(Qt::UserRole, device.serviceUrl);
    m_searchedTable->item(row, 0)->setData(Qt::UserRole + 1, device.discoveryType);
    
    updateSearchedGroupTitle();
}

void DeviceManageWidget::onAddDevice() {
    QList<QTableWidgetItem*> selected = m_searchedTable->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a device to add."));
        return;
    }
    
    int row = selected.first()->row();
    QString ip = m_searchedTable->item(row, 0)->text();
    QString name = m_searchedTable->item(row, 1)->text();
    QString serviceUrl = m_searchedTable->item(row, 0)->data(Qt::UserRole).toString();
    
    // Show dialog to get credentials
    bool ok;
    QString username = QInputDialog::getText(this, tr("Add Device"), 
        tr("Username:"), QLineEdit::Normal, "admin", &ok);
    if (!ok) return;
    
    QString password = QInputDialog::getText(this, tr("Add Device"), 
        tr("Password:"), QLineEdit::Password, "", &ok);
    if (!ok) return;
    
    if (!m_cameraManager || !m_onvifClient) {
        return;
    }

    QString model = m_searchedTable->item(row, 3)->text();
    QString discoveryType = m_searchedTable->item(row, 0)->data(Qt::UserRole + 1).toString();

    // 검색 결과를 그대로 저장하면 스트림 URL 을 모르는 채 추가되어 재생이 안 된다.
    // (또한 onvifPath 에 전체 URL 이 들어가 getOnvifUrl() 이 깨졌다.)
    // 더블클릭 경로와 같은 대화상자를 자격 증명이 채워진 상태로 열고 접속 테스트를 자동 실행한다.
    AddCameraDialog dialog(m_onvifClient, this);

    if (discoveryType == "ONVIF") {
        dialog.setDeviceInfo(ip, name, serviceUrl, model);
        dialog.setCredentials(username, password);
        dialog.startConnectionTest();
    } else {
        CameraInfo info;
        info.name = name.isEmpty() ? ip : name;
        info.model = model;
        info.ip = ip;
        info.port = 554;
        info.type = CameraType::RTSP;
        info.rtspUrl = QString("rtsp://%1:554/stream1").arg(ip);
        info.username = username;
        info.password = password;
        dialog.setCameraInfo(info);
    }

    if (dialog.exec() == QDialog::Accepted) {
        m_cameraManager->addCamera(dialog.getCameraInfo());
    }
}

void DeviceManageWidget::onModifyIP() {
    if (!m_onvifClient) {
        QMessageBox::warning(this, tr("Error"), tr("ONVIF client not available."));
        return;
    }
    
    QList<QTableWidgetItem*> selected = m_searchedTable->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a device to modify."));
        return;
    }
    
    int row = selected.first()->row();
    QString ip = m_searchedTable->item(row, 0)->text();
    QString discoveryType = m_searchedTable->item(row, 0)->data(Qt::UserRole + 1).toString();
    QString serviceUrl = m_searchedTable->item(row, 0)->data(Qt::UserRole).toString();
    
    // Only ONVIF devices support network modification
    if (discoveryType != "ONVIF") {
        QMessageBox::warning(this, tr("Not Supported"),
            tr("Network settings can only be modified for ONVIF devices.\n"
               "This device was discovered via %1.").arg(discoveryType));
        return;
    }
    
    // Get credentials
    bool ok;
    QString username = QInputDialog::getText(this, tr("Authentication Required"),
        tr("Username (admin access required):"), QLineEdit::Normal, "admin", &ok);
    if (!ok) return;
    
    QString password = QInputDialog::getText(this, tr("Authentication Required"),
        tr("Password:"), QLineEdit::Password, "", &ok);
    if (!ok) return;
    
    // Build device service URL if not available
    if (serviceUrl.isEmpty() || !serviceUrl.contains("onvif")) {
        serviceUrl = QString("http://%1:80/onvif/device_service").arg(ip);
    }
    
    NetworkSettingsDialog dialog(m_onvifClient, this);
    dialog.setDeviceInfo(ip, serviceUrl, username, password);
    dialog.exec();
    
    // Refresh the device list after closing
    QTimer::singleShot(500, this, &DeviceManageWidget::onStartSearch);
}

void DeviceManageWidget::onManualAdd() {
    if (!m_onvifClient || !m_cameraManager) return;
    
    AddCameraDialog dialog(m_onvifClient, this);
    if (dialog.exec() == QDialog::Accepted) {
        CameraInfo info = dialog.getCameraInfo();
        m_cameraManager->addCamera(info);
    }
}

void DeviceManageWidget::onDeleteDevice() {
    if (!m_cameraManager) return;
    
    QList<QTableWidgetItem*> selected = m_addedTable->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a device to delete."));
        return;
    }
    
    int row = selected.first()->row();
    QString deviceId = m_addedTable->item(row, 0)->data(Qt::UserRole).toString();
    
    int ret = QMessageBox::question(this, tr("Delete Device"),
        tr("Are you sure you want to delete this device?"),
        QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        m_cameraManager->removeCamera(deviceId);
    }
}

void DeviceManageWidget::onEditDevice() {
    if (!m_cameraManager || !m_onvifClient) return;
    
    QList<QTableWidgetItem*> selected = m_addedTable->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a device to edit."));
        return;
    }
    
    int row = selected.first()->row();
    QString deviceId = m_addedTable->item(row, 0)->data(Qt::UserRole).toString();
    
    CameraInfo info = m_cameraManager->getCamera(deviceId);
    if (info.id.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Camera not found."));
        return;
    }
    
    AddCameraDialog dialog(m_onvifClient, this);
    dialog.setCameraInfo(info);
    
    if (dialog.exec() == QDialog::Accepted) {
        CameraInfo updatedInfo = dialog.getCameraInfo();
        updatedInfo.id = deviceId;
        m_cameraManager->updateCamera(updatedInfo);
        refreshAddedDevices();
    }
}

void DeviceManageWidget::onSearchedDeviceDoubleClicked(int row, int column) {
    Q_UNUSED(column);
    
    if (!m_onvifClient || !m_cameraManager) return;
    
    QString ip = m_searchedTable->item(row, 0)->text();
    QString name = m_searchedTable->item(row, 1)->text();
    QString serviceUrl = m_searchedTable->item(row, 0)->data(Qt::UserRole).toString();
    QString discoveryType = m_searchedTable->item(row, 0)->data(Qt::UserRole + 1).toString();
    
    AddCameraDialog dialog(m_onvifClient, this);
    
    if (discoveryType == "ONVIF") {
        QString model = m_searchedTable->item(row, 3)->text();
        dialog.setDeviceInfo(ip, name, serviceUrl, model);
    } else {
        // For mDNS/UPnP devices, set as RTSP type with guessed URL
        QString model = m_searchedTable->item(row, 3)->text();
        CameraInfo info;
        info.name = name;
        info.model = model;
        info.ip = ip;
        info.port = 554;
        info.type = CameraType::RTSP;
        info.rtspUrl = QString("rtsp://%1:554/stream1").arg(ip);
        dialog.setCameraInfo(info);
    }
    
    if (dialog.exec() == QDialog::Accepted) {
        CameraInfo info = dialog.getCameraInfo();
        m_cameraManager->addCamera(info);
    }
}

void DeviceManageWidget::onAddedDeviceDoubleClicked(int row, int column) {
    Q_UNUSED(column);
    
    if (!m_cameraManager || !m_onvifClient) return;
    
    QString deviceId = m_addedTable->item(row, 0)->data(Qt::UserRole).toString();
    
    CameraInfo info = m_cameraManager->getCamera(deviceId);
    if (info.id.isEmpty()) return;
    
    AddCameraDialog dialog(m_onvifClient, this);
    dialog.setCameraInfo(info);
    
    if (dialog.exec() == QDialog::Accepted) {
        CameraInfo updatedInfo = dialog.getCameraInfo();
        updatedInfo.id = deviceId;
        m_cameraManager->updateCamera(updatedInfo);
        refreshAddedDevices();
    }
}

void DeviceManageWidget::refreshAddedDevices() {
    updateAddedDeviceTable();
}

void DeviceManageWidget::updateAddedDeviceTable() {
    if (!m_cameraManager) return;
    
    m_addedTable->setRowCount(0);
    
    QList<CameraInfo> cameras = m_cameraManager->getAllCameras();
    for (const CameraInfo& cam : cameras) {
        int row = m_addedTable->rowCount();
        m_addedTable->insertRow(row);
        
        QTableWidgetItem* nameItem = new QTableWidgetItem(cam.name);
        nameItem->setData(Qt::UserRole, cam.id);
        m_addedTable->setItem(row, 0, nameItem);
        
        m_addedTable->setItem(row, 1, new QTableWidgetItem(cam.ip));
        m_addedTable->setItem(row, 2, new QTableWidgetItem(cam.model));
        m_addedTable->setItem(row, 3, new QTableWidgetItem(cam.serialNumber));
        
        QString typeStr = (cam.type == CameraType::ONVIF) ? "ONVIF" : 
                          (cam.type == CameraType::RTSP) ? "RTSP" : "Generic";
        m_addedTable->setItem(row, 4, new QTableWidgetItem(typeStr));
        
        // Online = 둘 중 하나라도 열림. 하나만 실패하면 어느 쪽인지 표시. Offline = 둘 다 실패.
        QString stateStr;
        QColor stateColor;
        if (cam.status == CameraStatus::Online) {
            if (cam.mainStreamState == StreamState::Failed) {
                stateStr = tr("Online (main stream failed)");
                stateColor = QColor(200, 130, 0);
            } else if (cam.subStreamState == StreamState::Failed) {
                stateStr = tr("Online (sub stream failed)");
                stateColor = QColor(200, 130, 0);
            } else {
                stateStr = tr("Online");
                stateColor = QColor(0, 150, 0);
            }
        } else if (cam.status == CameraStatus::Offline) {
            stateStr = tr("Offline");
            stateColor = QColor(150, 0, 0);
        } else {
            stateStr = tr("Checking...");
            stateColor = QColor(128, 128, 128);
        }
        QTableWidgetItem* stateItem = new QTableWidgetItem(stateStr);
        stateItem->setForeground(stateColor);
        auto describe = [this](StreamState state, const QString& error) {
            if (state == StreamState::Ok) return tr("OK");
            if (state == StreamState::Failed) return error.isEmpty() ? tr("Failed") : tr("Failed: %1").arg(error);
            return tr("Checking...");
        };
        stateItem->setToolTip(tr("Main stream: %1\nSub stream: %2")
            .arg(describe(cam.mainStreamState, cam.mainStreamError))
            .arg(describe(cam.subStreamState, cam.subStreamError)));
        m_addedTable->setItem(row, 5, stateItem);
        
        m_addedTable->setItem(row, 6, new QTableWidgetItem(""));
    }
    
    updateAddedGroupTitle();
}

void DeviceManageWidget::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    refreshAddedDevices();
    refreshInterfaceList();
}

void DeviceManageWidget::refreshInterfaceList() {
    if (!m_interfaceCombo) {
        return;
    }
    const QString previous = m_interfaceCombo->currentData().toString();

    m_interfaceCombo->blockSignals(true);
    m_interfaceCombo->clear();
    m_interfaceCombo->addItem(tr("All interfaces"), QString());
    for (const LocalInterface& li : discoveryInterfaces()) {
        m_interfaceCombo->addItem(li.label(), li.address.toString());
    }
    int index = m_interfaceCombo->findData(previous);
    m_interfaceCombo->setCurrentIndex(index >= 0 ? index : 0);
    m_interfaceCombo->blockSignals(false);
}

QList<QHostAddress> DeviceManageWidget::selectedLocalAddresses() const {
    QList<QHostAddress> result;
    if (m_interfaceCombo) {
        const QString data = m_interfaceCombo->currentData().toString();
        if (!data.isEmpty()) {
            result.append(QHostAddress(data));
        }
    }
    return result;
}

void DeviceManageWidget::updateSearchedGroupTitle() {
    int count = m_searchedTable->rowCount();
    m_searchedGroup->setTitle(tr("Searched device (%1)").arg(count));
}

void DeviceManageWidget::updateAddedGroupTitle() {
    int count = m_addedTable->rowCount();
    m_addedGroup->setTitle(tr("Added device (%1)").arg(count));
}

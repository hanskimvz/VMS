#include "main_window.h"
#include "video_grid.h"
#include "camera_tree.h"
#include "playback_view.h"
#include "device_manage_widget.h"
#include "add_camera_dialog.h"
#include "camera_manager.h"
#include "onvif_client.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSettings>
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QPushButton>
#include <QFrame>
#include <QSplitter>
#include <QLineEdit>
#include <QStatusBar>
#include <QPainter>
#include <QShortcut>
#include <iostream>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_cameraManager(std::make_unique<CameraManager>(this))
    , m_onvifClient(std::make_unique<OnvifClient>(this))
{
    setWindowTitle("VMS - Video Management System");
    setMinimumSize(1280, 720);
    resize(1600, 900);
    
    applyDarkTheme();
    setupUi();
    setupConnections();
    loadSettings();
    
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/vms.db";
    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    m_cameraManager->initialize(dbPath);
    
    m_cameraTree->setCameraManager(m_cameraManager.get());
}

MainWindow::~MainWindow() {
    saveSettings();
}

void MainWindow::setupUi() {
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    setupHeaderBar();
    mainLayout->addWidget(findChild<QWidget*>("headerBar"));
    
    QSplitter* splitter = new QSplitter(Qt::Horizontal, centralWidget);
    splitter->setHandleWidth(1);
    
    setupSidePanel();
    splitter->addWidget(findChild<QWidget*>("sidePanel"));
    
    QWidget* rightArea = new QWidget(splitter);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightArea);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);
    
    setupCentralArea();
    rightLayout->addWidget(m_stackedWidget, 1);
    
    setupLayoutButtons();
    rightLayout->addWidget(m_layoutBar);
    
    splitter->addWidget(rightArea);
    splitter->setSizes({200, 1000});
    
    mainLayout->addWidget(splitter, 1);
    
    setCentralWidget(centralWidget);
    
    // 메뉴바 숨기기 (헤더바로 대체)
    menuBar()->hide();
}

void MainWindow::setupHeaderBar() {
    QWidget* headerBar = new QWidget(this);
    headerBar->setObjectName("headerBar");
    headerBar->setFixedHeight(70);
    headerBar->setStyleSheet(
        "QWidget#headerBar { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "stop:0 #1a1a2e, stop:1 #16213e); }"
    );
    
    QHBoxLayout* headerLayout = new QHBoxLayout(headerBar);
    headerLayout->setContentsMargins(15, 5, 15, 5);
    headerLayout->setSpacing(10);
    
    // Logo
    QLabel* logoLabel = new QLabel(headerBar);
    QPixmap logoPixmap(":/images/logo.png");
    if (!logoPixmap.isNull()) {
        logoLabel->setPixmap(logoPixmap.scaled(120, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        logoLabel->setText("VMS");
        logoLabel->setStyleSheet("color: #4da6ff; font-size: 24px; font-weight: bold;");
    }
    headerLayout->addWidget(logoLabel);
    
    headerLayout->addSpacing(30);
    
    // Tab buttons
    auto createTabButton = [this, headerBar](const QString& text, const QString& iconText) -> QToolButton* {
        QToolButton* btn = new QToolButton(headerBar);
        btn->setText(text);
        btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        btn->setFixedSize(90, 60);
        btn->setCheckable(true);
        btn->setAutoExclusive(true);
        btn->setStyleSheet(
            "QToolButton { "
            "   background: transparent; "
            "   border: none; "
            "   color: #aaaaaa; "
            "   font-size: 11px; "
            "   padding: 5px; "
            "} "
            "QToolButton:hover { "
            "   color: #ffffff; "
            "   background: rgba(255, 255, 255, 0.1); "
            "} "
            "QToolButton:checked { "
            "   color: #4da6ff; "
            "   border-bottom: 3px solid #4da6ff; "
            "}"
        );
        m_tabButtons.append(btn);
        return btn;
    };
    
    m_liveBtn = createTabButton(tr("Main View"), "▶");
    m_deviceManageBtn = createTabButton(tr("Device Manage"), "⚙");
    m_playbackBtn = createTabButton(tr("Playback"), "⏪");
    m_recordScheduleBtn = createTabButton(tr("Record Schedule"), "📅");
    m_settingsBtn = createTabButton(tr("Sys. Settings"), "⚙");
    
    headerLayout->addWidget(m_liveBtn);
    headerLayout->addWidget(m_deviceManageBtn);
    headerLayout->addWidget(m_playbackBtn);
    headerLayout->addWidget(m_recordScheduleBtn);
    headerLayout->addWidget(m_settingsBtn);
    
    headerLayout->addStretch();
    
    // Info button (오른쪽)
    QToolButton* infoBtn = new QToolButton(headerBar);
    infoBtn->setText("ℹ");
    infoBtn->setFixedSize(30, 30);
    infoBtn->setStyleSheet(
        "QToolButton { "
        "   background: transparent; "
        "   border: 1px solid #555; "
        "   border-radius: 15px; "
        "   color: #aaa; "
        "   font-size: 16px; "
        "}"
        "QToolButton:hover { background: rgba(255,255,255,0.1); }"
    );
    connect(infoBtn, &QToolButton::clicked, this, &MainWindow::onAbout);
    headerLayout->addWidget(infoBtn);
    
    m_liveBtn->setChecked(true);
}

void MainWindow::setupSidePanel() {
    QWidget* sidePanel = new QWidget(this);
    sidePanel->setObjectName("sidePanel");
    sidePanel->setMinimumWidth(180);
    sidePanel->setMaximumWidth(300);
    sidePanel->setStyleSheet("background-color: #252526;");
    
    QVBoxLayout* sideLayout = new QVBoxLayout(sidePanel);
    sideLayout->setContentsMargins(5, 5, 5, 5);
    sideLayout->setSpacing(5);
    
    // Search box (placeholder)
    QLineEdit* searchBox = new QLineEdit(sidePanel);
    searchBox->setPlaceholderText(tr("Search..."));
    searchBox->setStyleSheet(
        "QLineEdit { "
        "   background: #3c3c3c; "
        "   border: 1px solid #555; "
        "   border-radius: 3px; "
        "   padding: 5px; "
        "   color: white; "
        "}"
    );
    sideLayout->addWidget(searchBox);
    
    // Camera tree
    m_cameraTree = new CameraTree(sidePanel);
    sideLayout->addWidget(m_cameraTree, 1);
}

void MainWindow::setupCentralArea() {
    m_stackedWidget = new QStackedWidget(this);
    
    // Live View page
    m_liveGrid = new VideoGrid(m_stackedWidget);
    m_stackedWidget->addWidget(m_liveGrid);
    
    // Device Manage page
    m_deviceManageWidget = new DeviceManageWidget(m_stackedWidget);
    m_deviceManageWidget->setCameraManager(m_cameraManager.get());
    m_deviceManageWidget->setOnvifClient(m_onvifClient.get());
    m_stackedWidget->addWidget(m_deviceManageWidget);
    
    // Playback page
    m_playbackView = new PlaybackView(m_stackedWidget);
    m_stackedWidget->addWidget(m_playbackView);
    
    // Record Schedule page
    m_recordSchedulePage = new QWidget(m_stackedWidget);
    QLabel* recordLabel = new QLabel(tr("Record Schedule\n\nConfigure recording schedules here."), m_recordSchedulePage);
    recordLabel->setAlignment(Qt::AlignCenter);
    recordLabel->setStyleSheet("color: #888; font-size: 16px;");
    QVBoxLayout* recordLayout = new QVBoxLayout(m_recordSchedulePage);
    recordLayout->addWidget(recordLabel);
    m_stackedWidget->addWidget(m_recordSchedulePage);
    
    // Settings page
    m_settingsPage = new QWidget(m_stackedWidget);
    QLabel* settingsLabel = new QLabel(tr("System Settings\n\nConfigure system settings here."), m_settingsPage);
    settingsLabel->setAlignment(Qt::AlignCenter);
    settingsLabel->setStyleSheet("color: #888; font-size: 16px;");
    QVBoxLayout* settingsLayout = new QVBoxLayout(m_settingsPage);
    settingsLayout->addWidget(settingsLabel);
    m_stackedWidget->addWidget(m_settingsPage);
}

void MainWindow::setupLayoutButtons() {
    m_layoutBar = new QWidget(this);
    m_layoutBar->setFixedHeight(40);
    m_layoutBar->setStyleSheet("background-color: #1e1e1e;");
    
    QHBoxLayout* layoutBarLayout = new QHBoxLayout(m_layoutBar);
    layoutBarLayout->setContentsMargins(10, 5, 10, 5);
    layoutBarLayout->setSpacing(3);
    
    layoutBarLayout->addStretch();
    
    // 그리드 아이콘을 그리는 커스텀 버튼 생성
    auto createGridIcon = [](int rows, int cols) -> QPixmap {
        int size = 24;
        int padding = 3;
        int cellSpacing = 1;
        
        QPixmap pixmap(size, size);
        pixmap.fill(Qt::transparent);
        
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        
        int availableSize = size - 2 * padding;
        int cellWidth = (availableSize - (cols - 1) * cellSpacing) / cols;
        int cellHeight = (availableSize - (rows - 1) * cellSpacing) / rows;
        
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(150, 150, 150));
        
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                int x = padding + c * (cellWidth + cellSpacing);
                int y = padding + r * (cellHeight + cellSpacing);
                painter.drawRect(x, y, cellWidth, cellHeight);
            }
        }
        
        return pixmap;
    };
    
    // 1+7 레이아웃 아이콘 (큰 화면 + 오른쪽 작은 화면들)
    auto createMainPlusIcon = []() -> QPixmap {
        int size = 24;
        int padding = 3;
        int cellSpacing = 1;
        
        QPixmap pixmap(size, size);
        pixmap.fill(Qt::transparent);
        
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        
        int availableSize = size - 2 * padding;
        
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(150, 150, 150));
        
        // 왼쪽 큰 화면 (3/4 너비, 전체 높이)
        int mainWidth = availableSize * 3 / 4 - cellSpacing;
        int mainHeight = availableSize;
        painter.drawRect(padding, padding, mainWidth, mainHeight);
        
        // 오른쪽 작은 화면들 (4개)
        int smallWidth = availableSize / 4;
        int smallHeight = (availableSize - 3 * cellSpacing) / 4;
        int smallX = padding + mainWidth + cellSpacing;
        
        for (int i = 0; i < 4; ++i) {
            int smallY = padding + i * (smallHeight + cellSpacing);
            painter.drawRect(smallX, smallY, smallWidth, smallHeight);
        }
        
        return pixmap;
    };
    
    auto createLayoutBtn = [this, createGridIcon](int rows, int cols, int layout) -> QPushButton* {
        QPushButton* btn = new QPushButton(m_layoutBar);
        btn->setFixedSize(32, 32);
        btn->setIcon(QIcon(createGridIcon(rows, cols)));
        btn->setIconSize(QSize(24, 24));
        btn->setStyleSheet(
            "QPushButton { "
            "   background: #3c3c3c; "
            "   border: 1px solid #555; "
            "   border-radius: 3px; "
            "}"
            "QPushButton:hover { background: #4a4a4a; border-color: #7a7a7a; }"
            "QPushButton:pressed { background: #007acc; }"
        );
        connect(btn, &QPushButton::clicked, [this, layout]() { onLayoutChanged(layout); });
        return btn;
    };
    
    auto createSpecialLayoutBtn = [this](const QPixmap& icon, int layout) -> QPushButton* {
        QPushButton* btn = new QPushButton(m_layoutBar);
        btn->setFixedSize(32, 32);
        btn->setIcon(QIcon(icon));
        btn->setIconSize(QSize(24, 24));
        btn->setStyleSheet(
            "QPushButton { "
            "   background: #3c3c3c; "
            "   border: 1px solid #555; "
            "   border-radius: 3px; "
            "}"
            "QPushButton:hover { background: #4a4a4a; border-color: #7a7a7a; }"
            "QPushButton:pressed { background: #007acc; }"
        );
        connect(btn, &QPushButton::clicked, [this, layout]() { onLayoutChanged(layout); });
        return btn;
    };
    
    layoutBarLayout->addWidget(createLayoutBtn(1, 1, 1));                  // 1x1
    layoutBarLayout->addWidget(createLayoutBtn(2, 2, 4));                  // 2x2
    layoutBarLayout->addWidget(createSpecialLayoutBtn(createMainPlusIcon(), 8));  // 1+7
    layoutBarLayout->addWidget(createLayoutBtn(3, 3, 9));                  // 3x3
    layoutBarLayout->addWidget(createLayoutBtn(4, 4, 16));                 // 4x4
}

void MainWindow::setupConnections() {
    connect(m_liveBtn, &QToolButton::clicked, this, &MainWindow::onShowLiveView);
    connect(m_deviceManageBtn, &QToolButton::clicked, this, &MainWindow::onShowDeviceManage);
    connect(m_playbackBtn, &QToolButton::clicked, this, &MainWindow::onShowPlayback);
    connect(m_recordScheduleBtn, &QToolButton::clicked, this, &MainWindow::onShowRecordSchedule);
    connect(m_settingsBtn, &QToolButton::clicked, this, &MainWindow::onShowSettings);
    
    connect(m_cameraTree, &CameraTree::cameraSelected,
            this, &MainWindow::onCameraSelected);
    connect(m_cameraTree, &CameraTree::cameraDoubleClicked,
            this, &MainWindow::onCameraDoubleClicked);
    
    connect(m_onvifClient.get(), &OnvifClient::deviceDiscovered,
            [this](const OnvifDevice& device) {
                statusBar()->showMessage(tr("Found: %1 (%2)").arg(device.name).arg(device.address), 3000);
            });
    
    QShortcut* statsShortcut = new QShortcut(QKeySequence("F2"), this);
    connect(statsShortcut, &QShortcut::activated, [this]() {
        if (m_liveGrid) {
            m_liveGrid->toggleStats();
            statusBar()->showMessage(tr("Stream statistics toggled (F2)"), 2000);
        }
    });
}

void MainWindow::setActiveTab(QToolButton* button) {
    for (QToolButton* btn : m_tabButtons) {
        btn->setChecked(btn == button);
    }
}

void MainWindow::onAddCamera() {
    AddCameraDialog dialog(m_onvifClient.get(), this);
    
    if (dialog.exec() == QDialog::Accepted) {
        CameraInfo info = dialog.getCameraInfo();
        QString id = m_cameraManager->addCamera(info);
        statusBar()->showMessage(tr("Camera added: %1").arg(info.name), 3000);
    }
}

void MainWindow::onDiscoverCameras() {
    statusBar()->showMessage(tr("Discovering cameras..."));
    m_onvifClient->discover(5000);
}

void MainWindow::onCameraSelected(const QString& cameraId) {
    m_selectedCameraId = cameraId;
    CameraInfo info = m_cameraManager->getCamera(cameraId);
    statusBar()->showMessage(tr("Selected: %1").arg(info.name), 2000);
}

void MainWindow::onCameraDoubleClicked(const QString& cameraId) {
    CameraInfo info = m_cameraManager->getCamera(cameraId);
    
    bool useSubStream = shouldUseSubStream();
    
    if (!m_cameraManager->startStream(cameraId, useSubStream)) {
        statusBar()->showMessage(tr("Failed to connect to %1").arg(info.name), 3000);
        return;
    }
    
    StreamReceiver* receiver = m_cameraManager->getStreamReceiver(cameraId);
    if (receiver) {
        m_liveGrid->addStream(cameraId, info.name, receiver);
        QString streamType = useSubStream ? tr("sub-stream") : tr("main-stream");
        statusBar()->showMessage(tr("Connected: %1 (%2)").arg(info.name).arg(streamType), 3000);
        
        // Live view로 전환
        onShowLiveView();
    }
}

void MainWindow::onLayoutChanged(int layout) {
    m_liveGrid->setLayout(layout);
    
    updateStreamsForLayout();
    
    statusBar()->showMessage(tr("Layout: %1 cells").arg(layout), 2000);
}

void MainWindow::onShowLiveView() {
    setActiveTab(m_liveBtn);
    m_stackedWidget->setCurrentWidget(m_liveGrid);
}

void MainWindow::onShowDeviceManage() {
    setActiveTab(m_deviceManageBtn);
    m_stackedWidget->setCurrentWidget(m_deviceManageWidget);
}

void MainWindow::onShowPlayback() {
    setActiveTab(m_playbackBtn);
    m_stackedWidget->setCurrentWidget(m_playbackView);
}

void MainWindow::onShowRecordSchedule() {
    setActiveTab(m_recordScheduleBtn);
    m_stackedWidget->setCurrentWidget(m_recordSchedulePage);
}

void MainWindow::onShowSettings() {
    setActiveTab(m_settingsBtn);
    m_stackedWidget->setCurrentWidget(m_settingsPage);
}

void MainWindow::onAbout() {
    QMessageBox::about(this, tr("About VMS"),
        tr("<h3>VMS - Video Management System</h3>"
           "<p>Version 1.0.0</p>"
           "<p>A modern IP camera management system built with Qt6 and FFmpeg.</p>"));
}

void MainWindow::loadSettings() {
    QSettings settings("VMS", "VMS");
    
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
    
    int gridLayout = settings.value("gridLayout", 4).toInt();
    if (m_liveGrid) {
        m_liveGrid->setLayout(gridLayout);
    }
}

void MainWindow::saveSettings() {
    QSettings settings("VMS", "VMS");
    settings.setValue("geometry", saveGeometry());
    
    if (m_liveGrid) {
        settings.setValue("gridLayout", m_liveGrid->layout());
    }
}

void MainWindow::applyDarkTheme() {
    qApp->setStyle("Fusion");
    
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(30, 30, 30));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(35, 35, 35));
    darkPalette.setColor(QPalette::ToolTipBase, QColor(45, 45, 48));
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(45, 45, 48));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    
    qApp->setPalette(darkPalette);
    
    qApp->setStyleSheet(
        "QToolTip { color: #ffffff; background-color: #2d2d30; border: 1px solid #3f3f46; }"
        "QMenu { background-color: #2d2d30; border: 1px solid #3f3f46; }"
        "QMenu::item:selected { background-color: #3e3e42; }"
        "QStatusBar { background: #1e1e1e; color: #888; }"
        "QScrollBar:vertical { background: #2a2a2a; width: 12px; }"
        "QScrollBar::handle:vertical { background: #555; border-radius: 6px; min-height: 20px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
    );
}

bool MainWindow::shouldUseSubStream() const {
    if (!m_liveGrid) {
        return false;
    }
    
    int layout = m_liveGrid->layout();
    return layout > 1;
}

void MainWindow::updateStreamsForLayout() {
    if (!m_liveGrid || !m_cameraManager) {
        return;
    }
    
    bool useSubStream = shouldUseSubStream();
    
    QList<CameraInfo> cameras = m_cameraManager->getAllCameras();
    for (const CameraInfo& camera : cameras) {
        StreamReceiver* receiver = m_cameraManager->getStreamReceiver(camera.id);
        if (receiver) {
            bool currentUsingSub = m_cameraManager->isUsingSubStream(camera.id);
            if (currentUsingSub != useSubStream) {
                if (m_cameraManager->restartStream(camera.id, useSubStream)) {
                    StreamReceiver* newReceiver = m_cameraManager->getStreamReceiver(camera.id);
                    if (newReceiver) {
                        m_liveGrid->addStream(camera.id, camera.name, newReceiver);
                    }
                }
            }
        }
    }
}

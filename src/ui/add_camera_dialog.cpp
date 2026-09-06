#include "add_camera_dialog.h"
#include "onvif_client.h"
#include "stream_receiver.h"
#include "video_widget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QScrollArea>
#include <QSplitter>
#include <QTime>
#include <QThread>

AddCameraDialog::AddCameraDialog(OnvifClient* onvifClient, QWidget* parent)
    : QDialog(parent)
    , m_onvifClient(onvifClient)
{
    setWindowTitle(tr("Add Camera"));
    setMinimumSize(700, 600);
    
    setupUi();
    setupConnections();
    updateFieldsVisibility();
}

AddCameraDialog::~AddCameraDialog() {
    stopPreview();
}

void AddCameraDialog::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    
    QHBoxLayout* contentLayout = new QHBoxLayout();
    
    // Left side - Settings
    QWidget* leftWidget = new QWidget(this);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(10);
    
    // Basic Information
    QGroupBox* basicGroup = new QGroupBox(tr("Basic Information"), leftWidget);
    QFormLayout* basicLayout = new QFormLayout(basicGroup);
    basicLayout->setSpacing(8);
    
    m_nameEdit = new QLineEdit(basicGroup);
    m_nameEdit->setPlaceholderText(tr("Enter camera name"));
    basicLayout->addRow(tr("Name:"), m_nameEdit);
    
    m_modelEdit = new QLineEdit(basicGroup);
    m_modelEdit->setPlaceholderText(tr("Device model (auto-detected)"));
    m_modelEdit->setReadOnly(true);
    m_modelEdit->setStyleSheet("QLineEdit { background-color: #f0f0f0; color: #666; }");
    basicLayout->addRow(tr("Model:"), m_modelEdit);
    
    m_serialNumberEdit = new QLineEdit(basicGroup);
    m_serialNumberEdit->setPlaceholderText(tr("Serial number (auto-detected)"));
    m_serialNumberEdit->setReadOnly(true);
    m_serialNumberEdit->setStyleSheet("QLineEdit { background-color: #f0f0f0; color: #666; }");
    basicLayout->addRow(tr("Serial Number:"), m_serialNumberEdit);
    
    m_manufacturerEdit = new QLineEdit(basicGroup);
    m_manufacturerEdit->setPlaceholderText(tr("Manufacturer (auto-detected)"));
    m_manufacturerEdit->setReadOnly(true);
    m_manufacturerEdit->setStyleSheet("QLineEdit { background-color: #f0f0f0; color: #666; }");
    basicLayout->addRow(tr("Manufacturer:"), m_manufacturerEdit);
    
    m_typeCombo = new QComboBox(basicGroup);
    m_typeCombo->addItem(tr("ONVIF"), static_cast<int>(CameraType::ONVIF));
    m_typeCombo->addItem(tr("RTSP"), static_cast<int>(CameraType::RTSP));
    basicLayout->addRow(tr("Type:"), m_typeCombo);
    
    leftLayout->addWidget(basicGroup);
    
    // ONVIF Settings
    m_onvifGroup = new QGroupBox(tr("ONVIF Connection"), leftWidget);
    QFormLayout* onvifLayout = new QFormLayout(m_onvifGroup);
    onvifLayout->setSpacing(8);
    
    m_ipEdit = new QLineEdit(m_onvifGroup);
    m_ipEdit->setPlaceholderText("192.168.1.100");
    onvifLayout->addRow(tr("IP Address:"), m_ipEdit);
    
    m_onvifPortSpin = new QSpinBox(m_onvifGroup);
    m_onvifPortSpin->setRange(1, 65535);
    m_onvifPortSpin->setValue(80);
    onvifLayout->addRow(tr("Port:"), m_onvifPortSpin);
    
    m_usernameEdit = new QLineEdit(m_onvifGroup);
    m_usernameEdit->setPlaceholderText("admin");
    onvifLayout->addRow(tr("Username:"), m_usernameEdit);
    
    m_passwordEdit = new QLineEdit(m_onvifGroup);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    onvifLayout->addRow(tr("Password:"), m_passwordEdit);
    
    m_onvifPathEdit = new QLineEdit(m_onvifGroup);
    m_onvifPathEdit->setText("/onvif/device_service");
    onvifLayout->addRow(tr("ONVIF Path:"), m_onvifPathEdit);
    
    leftLayout->addWidget(m_onvifGroup);
    
    // Profile Selection
    m_profileGroup = new QGroupBox(tr("Stream Profiles"), leftWidget);
    QVBoxLayout* profileLayout = new QVBoxLayout(m_profileGroup);
    
    // Get Profiles button
    QHBoxLayout* profileBtnLayout = new QHBoxLayout();
    profileBtnLayout->addStretch();
    m_getProfilesBtn = new QPushButton(tr("Get Profiles"), m_profileGroup);
    profileBtnLayout->addWidget(m_getProfilesBtn);
    profileLayout->addLayout(profileBtnLayout);
    
    // Main Stream
    QFormLayout* mainStreamLayout = new QFormLayout();
    QLabel* mainLabel = new QLabel(tr("<b>Main Stream</b> (High Quality - for 1-8 channels):"), m_profileGroup);
    profileLayout->addWidget(mainLabel);
    
    m_mainProfileCombo = new QComboBox(m_profileGroup);
    m_mainProfileCombo->setEnabled(false);
    mainStreamLayout->addRow(tr("Profile:"), m_mainProfileCombo);
    
    m_mainProfileInfoLabel = new QLabel(m_profileGroup);
    m_mainProfileInfoLabel->setStyleSheet("color: #888;");
    mainStreamLayout->addRow("", m_mainProfileInfoLabel);
    
    m_mainRtspUrlDisplay = new QLineEdit(m_profileGroup);
    m_mainRtspUrlDisplay->setReadOnly(true);
    m_mainRtspUrlDisplay->setPlaceholderText(tr("Main RTSP URL"));
    mainStreamLayout->addRow(tr("URL:"), m_mainRtspUrlDisplay);
    profileLayout->addLayout(mainStreamLayout);
    
    // Sub Stream
    profileLayout->addSpacing(10);
    QLabel* subLabel = new QLabel(tr("<b>Sub Stream</b> (Low Quality - for 9+ channels):"), m_profileGroup);
    profileLayout->addWidget(subLabel);
    
    QFormLayout* subStreamLayout = new QFormLayout();
    m_subProfileCombo = new QComboBox(m_profileGroup);
    m_subProfileCombo->setEnabled(false);
    subStreamLayout->addRow(tr("Profile:"), m_subProfileCombo);
    
    m_subProfileInfoLabel = new QLabel(m_profileGroup);
    m_subProfileInfoLabel->setStyleSheet("color: #888;");
    subStreamLayout->addRow("", m_subProfileInfoLabel);
    
    m_subRtspUrlDisplay = new QLineEdit(m_profileGroup);
    m_subRtspUrlDisplay->setReadOnly(true);
    m_subRtspUrlDisplay->setPlaceholderText(tr("Sub RTSP URL"));
    subStreamLayout->addRow(tr("URL:"), m_subRtspUrlDisplay);
    profileLayout->addLayout(subStreamLayout);
    
    leftLayout->addWidget(m_profileGroup);
    
    // RTSP Settings (alternative)
    m_rtspGroup = new QGroupBox(tr("RTSP Settings"), leftWidget);
    QFormLayout* rtspLayout = new QFormLayout(m_rtspGroup);
    rtspLayout->setSpacing(8);
    
    m_rtspMainUrlEdit = new QLineEdit(m_rtspGroup);
    m_rtspMainUrlEdit->setPlaceholderText("rtsp://192.168.1.100:554/stream1");
    rtspLayout->addRow(tr("Main URL:"), m_rtspMainUrlEdit);
    
    m_rtspSubUrlEdit = new QLineEdit(m_rtspGroup);
    m_rtspSubUrlEdit->setPlaceholderText("rtsp://192.168.1.100:554/stream2 (optional)");
    rtspLayout->addRow(tr("Sub URL:"), m_rtspSubUrlEdit);
    
    m_rtspUsernameEdit = new QLineEdit(m_rtspGroup);
    m_rtspUsernameEdit->setPlaceholderText(tr("(optional)"));
    rtspLayout->addRow(tr("Username:"), m_rtspUsernameEdit);
    
    m_rtspPasswordEdit = new QLineEdit(m_rtspGroup);
    m_rtspPasswordEdit->setEchoMode(QLineEdit::Password);
    rtspLayout->addRow(tr("Password:"), m_rtspPasswordEdit);
    
    leftLayout->addWidget(m_rtspGroup);
    
    leftLayout->addStretch();
    
    contentLayout->addWidget(leftWidget, 1);
    
    // Right side - Preview
    QWidget* rightWidget = new QWidget(this);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    
    m_previewGroup = new QGroupBox(tr("Preview"), rightWidget);
    QVBoxLayout* previewLayout = new QVBoxLayout(m_previewGroup);
    
    m_previewWidget = new VideoWidget(m_previewGroup);
    m_previewWidget->setMinimumSize(320, 240);
    m_previewWidget->setCameraName(tr("No preview"));
    previewLayout->addWidget(m_previewWidget);
    
    m_testButton = new QPushButton(tr("Test Connection"), m_previewGroup);
    previewLayout->addWidget(m_testButton);
    
    rightLayout->addWidget(m_previewGroup);
    
    // Log
    QGroupBox* logGroup = new QGroupBox(tr("Connection Log"), rightWidget);
    QVBoxLayout* logLayout = new QVBoxLayout(logGroup);
    
    m_logEdit = new QTextEdit(logGroup);
    m_logEdit->setReadOnly(true);
    m_logEdit->setMaximumHeight(100);
    m_logEdit->setStyleSheet("QTextEdit { background: #1e1e1e; color: #aaa; font-family: monospace; font-size: 10px; }");
    logLayout->addWidget(m_logEdit);
    
    rightLayout->addWidget(logGroup);
    
    contentLayout->addWidget(rightWidget, 1);
    
    mainLayout->addLayout(contentLayout, 1);
    
    // Bottom buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_okButton = new QPushButton(tr("OK"), this);
    m_okButton->setDefault(true);
    m_okButton->setMinimumWidth(80);
    buttonLayout->addWidget(m_okButton);
    
    m_cancelButton = new QPushButton(tr("Cancel"), this);
    m_cancelButton->setMinimumWidth(80);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
}

void AddCameraDialog::setupConnections() {
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AddCameraDialog::onTypeChanged);
    connect(m_testButton, &QPushButton::clicked, this, &AddCameraDialog::onTestConnection);
    connect(m_getProfilesBtn, &QPushButton::clicked, this, &AddCameraDialog::onGetProfiles);
    connect(m_mainProfileCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AddCameraDialog::onMainProfileChanged);
    connect(m_subProfileCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AddCameraDialog::onSubProfileChanged);
    connect(m_okButton, &QPushButton::clicked, this, &AddCameraDialog::validate);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    // ONVIF signals
    connect(m_onvifClient, &OnvifClient::capabilitiesReceived,
            this, &AddCameraDialog::onCapabilitiesReceived);
    connect(m_onvifClient, &OnvifClient::deviceInformationReceived,
            this, &AddCameraDialog::onDeviceInformationReceived);
    connect(m_onvifClient, &OnvifClient::profilesReceived,
            this, &AddCameraDialog::onProfilesReceived);
    connect(m_onvifClient, &OnvifClient::streamUriReceived,
            this, &AddCameraDialog::onStreamUriReceived);
    connect(m_onvifClient, &OnvifClient::error,
            this, &AddCameraDialog::onOnvifError);
}

void AddCameraDialog::onTypeChanged(int index) {
    Q_UNUSED(index);
    stopPreview();
    updateFieldsVisibility();
}

void AddCameraDialog::updateFieldsVisibility() {
    CameraType type = static_cast<CameraType>(m_typeCombo->currentData().toInt());
    
    bool isOnvif = (type == CameraType::ONVIF);
    
    m_onvifGroup->setVisible(isOnvif);
    m_profileGroup->setVisible(isOnvif);
    m_rtspGroup->setVisible(!isOnvif);
}

void AddCameraDialog::onTestConnection() {
    stopPreview();
    m_logEdit->clear();
    
    CameraType type = static_cast<CameraType>(m_typeCombo->currentData().toInt());
    
    // Stop any existing preview first to free camera resources
    stopPreview();
    
    if (type == CameraType::ONVIF) {
        QString ip = m_ipEdit->text().trimmed();
        int port = m_onvifPortSpin->value();
        QString path = m_onvifPathEdit->text().trimmed();
        
        if (ip.isEmpty()) {
            QMessageBox::warning(this, tr("Error"), tr("Please enter IP address."));
            return;
        }
        
        if (path.isEmpty()) {
            path = "/onvif/device_service";
        }
        if (!path.startsWith('/')) {
            path = "/" + path;
        }
        
        // Always rebuild URL from current form values
        m_deviceServiceUrl = QString("http://%1:%2%3").arg(ip).arg(port).arg(path);
        m_mediaServiceUrl.clear(); // Reset media URL
        m_fallbackMediaUrls.clear();
        m_useGetServices = true; // Try GetServices first
        
        appendLog(tr("Connecting to %1...").arg(m_deviceServiceUrl));
        
        m_onvifClient->setCredentials(m_usernameEdit->text(), m_passwordEdit->text());
        m_onvifClient->getServices(m_deviceServiceUrl);
        
        m_testButton->setEnabled(false);
        m_testButton->setText(tr("Connecting..."));
        
    } else {
        QString url = m_rtspMainUrlEdit->text().trimmed();
        if (url.isEmpty()) {
            QMessageBox::warning(this, tr("Error"), tr("Please enter RTSP URL."));
            return;
        }
        
        appendLog(tr("Connecting to %1...").arg(url));
        startPreview(url);
    }
}

void AddCameraDialog::onGetProfiles() {
    if (m_mediaServiceUrl.isEmpty()) {
        // First get capabilities
        onTestConnection();
        return;
    }
    
    appendLog(tr("Getting profiles from %1...").arg(m_mediaServiceUrl));
    m_getProfilesBtn->setEnabled(false);
    m_getProfilesBtn->setText(tr("Loading..."));
    
    m_onvifClient->setCredentials(m_usernameEdit->text(), m_passwordEdit->text());
    m_onvifClient->getProfiles(m_mediaServiceUrl);
}

void AddCameraDialog::onMainProfileChanged(int index) {
    if (index < 0 || index >= m_profiles.size()) return;
    
    const OnvifProfile& profile = m_profiles[index];
    m_mainProfileToken = profile.token;
    
    QString info = QString("%1x%2 @ %3fps, %4")
        .arg(profile.width)
        .arg(profile.height)
        .arg(profile.fps)
        .arg(profile.encoding);
    m_mainProfileInfoLabel->setText(info);
    
    // Get stream URI for this profile
    if (!m_mediaServiceUrl.isEmpty()) {
        appendLog(tr("Getting Main stream URI for %1...").arg(profile.name));
        m_pendingStreamUriRequests++;
        
        QTimer::singleShot(500, this, [this, profile]() {
            if (!m_mediaServiceUrl.isEmpty()) {
                m_onvifClient->getStreamUri(m_mediaServiceUrl, profile.token);
            }
        });
    }
}

void AddCameraDialog::onSubProfileChanged(int index) {
    if (index < 0 || index >= m_profiles.size()) return;
    
    const OnvifProfile& profile = m_profiles[index];
    m_subProfileToken = profile.token;
    
    QString info = QString("%1x%2 @ %3fps, %4")
        .arg(profile.width)
        .arg(profile.height)
        .arg(profile.fps)
        .arg(profile.encoding);
    m_subProfileInfoLabel->setText(info);
    
    // Get stream URI for this profile
    if (!m_mediaServiceUrl.isEmpty()) {
        appendLog(tr("Getting Sub stream URI for %1...").arg(profile.name));
        m_pendingStreamUriRequests++;
        
        QTimer::singleShot(800, this, [this, profile]() {
            if (!m_mediaServiceUrl.isEmpty()) {
                m_onvifClient->getStreamUri(m_mediaServiceUrl, profile.token);
            }
        });
    }
}

void AddCameraDialog::onCapabilitiesReceived(const OnvifCapabilities& capabilities) {
    m_testButton->setEnabled(true);
    m_testButton->setText(tr("Test Connection"));
    
    appendLog(tr("Capabilities/Services received:"));
    appendLog(tr("  Media: %1").arg(capabilities.mediaServiceUrl));
    if (!capabilities.ptzServiceUrl.isEmpty()) {
        appendLog(tr("  PTZ: %1").arg(capabilities.ptzServiceUrl));
    }
    
    m_mediaServiceUrl = capabilities.mediaServiceUrl;
    
    // Check if media URL is empty
    bool needsFallback = m_mediaServiceUrl.isEmpty();
    
    if (needsFallback) {
        // If GetServices returned nothing useful, try GetCapabilities
        if (m_useGetServices) {
            m_useGetServices = false;
            appendLog(tr("GetServices returned no media URL, trying GetCapabilities..."));
            m_onvifClient->getCapabilities(m_deviceServiceUrl);
            return;
        }
        
        // Use device_service as media URL (some cameras use single endpoint)
        m_mediaServiceUrl = m_deviceServiceUrl;
        appendLog(tr("Using device service URL for media: %1").arg(m_mediaServiceUrl));
    }
    
    // Some cameras use single endpoint for all services - that's valid!
    // Only set up fallback URLs if device_service doesn't work
    if (m_mediaServiceUrl.contains("device_service")) {
        QUrl deviceUrl(m_deviceServiceUrl);
        QString baseUrl = QString("http://%1:%2").arg(deviceUrl.host()).arg(deviceUrl.port(80));
        
    }
    
    // Get device information (manufacturer, model, serial number)
    appendLog(tr("Getting device information..."));
    QTimer::singleShot(300, this, [this]() {
        m_onvifClient->getDeviceInformation(m_deviceServiceUrl);
    });
    
    appendLog(tr("Getting profiles from: %1").arg(m_mediaServiceUrl));
    
    // Automatically get profiles (with delay to avoid overwhelming camera)
    QTimer::singleShot(800, this, [this]() {
        onGetProfiles();
    });
}

void AddCameraDialog::onDeviceInformationReceived(const OnvifDeviceInfo& info) {
    appendLog(tr("Device information received:"));
    appendLog(tr("  Manufacturer: %1").arg(info.manufacturer));
    appendLog(tr("  Model: %1").arg(info.model));
    appendLog(tr("  Serial Number: %1").arg(info.serialNumber));
    appendLog(tr("  Firmware: %1").arg(info.firmwareVersion));
    
    // Update UI fields
    if (!info.model.isEmpty()) {
        m_modelEdit->setText(info.model);
    }
    if (!info.serialNumber.isEmpty()) {
        m_serialNumberEdit->setText(info.serialNumber);
    }
    if (!info.manufacturer.isEmpty()) {
        m_manufacturerEdit->setText(info.manufacturer);
    }
}

void AddCameraDialog::onProfilesReceived(const QList<OnvifProfile>& profiles) {
    m_getProfilesBtn->setEnabled(true);
    m_getProfilesBtn->setText(tr("Get Profiles"));
    
    m_profiles = profiles;
    m_mainProfileCombo->clear();
    m_subProfileCombo->clear();
    m_mainProfileCombo->setEnabled(!profiles.isEmpty());
    m_subProfileCombo->setEnabled(!profiles.isEmpty());
    
    appendLog(tr("Found %1 profile(s):").arg(profiles.size()));
    
    // Sort profiles by resolution (highest first)
    QList<OnvifProfile> sortedProfiles = profiles;
    std::sort(sortedProfiles.begin(), sortedProfiles.end(), [](const OnvifProfile& a, const OnvifProfile& b) {
        return (a.width * a.height) > (b.width * b.height);
    });
    
    for (const OnvifProfile& profile : sortedProfiles) {
        QString displayText = QString("%1 (%2x%3)")
            .arg(profile.name)
            .arg(profile.width)
            .arg(profile.height);
        m_mainProfileCombo->addItem(displayText, profile.token);
        m_subProfileCombo->addItem(displayText, profile.token);
        
        appendLog(tr("  - %1: %2x%3 %4fps %5")
            .arg(profile.name)
            .arg(profile.width)
            .arg(profile.height)
            .arg(profile.fps)
            .arg(profile.encoding));
    }
    
    if (!sortedProfiles.isEmpty()) {
        // Set Main to highest resolution (index 0)
        m_mainProfileCombo->blockSignals(true);
        m_mainProfileCombo->setCurrentIndex(0);
        m_mainProfileCombo->blockSignals(false);
        
        const OnvifProfile& mainProfile = sortedProfiles[0];
        m_mainProfileToken = mainProfile.token;
        QString mainInfo = QString("%1x%2 @ %3fps, %4")
            .arg(mainProfile.width).arg(mainProfile.height)
            .arg(mainProfile.fps).arg(mainProfile.encoding);
        m_mainProfileInfoLabel->setText(mainInfo);
        
        // Set Sub to lowest resolution (last index) or second if only 2
        int subIndex = sortedProfiles.size() > 1 ? sortedProfiles.size() - 1 : 0;
        m_subProfileCombo->blockSignals(true);
        m_subProfileCombo->setCurrentIndex(subIndex);
        m_subProfileCombo->blockSignals(false);
        
        const OnvifProfile& subProfile = sortedProfiles[subIndex];
        m_subProfileToken = subProfile.token;
        QString subInfo = QString("%1x%2 @ %3fps, %4")
            .arg(subProfile.width).arg(subProfile.height)
            .arg(subProfile.fps).arg(subProfile.encoding);
        m_subProfileInfoLabel->setText(subInfo);
        
        // Get stream URIs for both profiles
        m_pendingStreamUriRequests = 2;
        appendLog(tr("Getting stream URIs..."));
        
        QTimer::singleShot(500, this, [this, mainProfile]() {
            if (!m_mediaServiceUrl.isEmpty()) {
                m_onvifClient->getStreamUri(m_mediaServiceUrl, mainProfile.token);
            }
        });
        
        QTimer::singleShot(1500, this, [this, subProfile]() {
            if (!m_mediaServiceUrl.isEmpty()) {
                m_onvifClient->getStreamUri(m_mediaServiceUrl, subProfile.token);
            }
        });
    }
}

void AddCameraDialog::onStreamUriReceived(const QString& profileToken, const QString& uri) {
    QString username = m_usernameEdit->text();
    QString password = m_passwordEdit->text();
    
    QString displayUri = uri;
    QString connectUri = uri;
    
    if (!username.isEmpty()) {
        QUrl url(uri);
        url.setUserName(username);
        url.setPassword(password);
        connectUri = url.toString();
    }
    
    // Determine if this is Main or Sub stream
    if (profileToken == m_mainProfileToken) {
        m_mainRtspUrl = uri;
        m_mainRtspUrlDisplay->setText(displayUri);
        appendLog(tr("Main Stream URI: %1").arg(displayUri));
        
        // Start preview with main stream
        m_pendingStreamUriRequests--;
        if (m_pendingStreamUriRequests <= 0) {
            appendLog(tr("Starting preview..."));
            startPreview(connectUri);
        }
    } else if (profileToken == m_subProfileToken) {
        m_subRtspUrl = uri;
        m_subRtspUrlDisplay->setText(displayUri);
        appendLog(tr("Sub Stream URI: %1").arg(displayUri));
        m_pendingStreamUriRequests--;
    } else {
        // Unknown profile, treat as main
        m_mainRtspUrl = uri;
        m_mainRtspUrlDisplay->setText(displayUri);
        appendLog(tr("Stream URI: %1").arg(displayUri));
        
        appendLog(tr("Starting preview..."));
        startPreview(connectUri);
    }
}

void AddCameraDialog::onOnvifError(const QString& message) {
    appendLog(tr("Error: %1").arg(message));
    
    // If we have fallback URLs to try for media service
    if (message.contains("Not Found") && !m_fallbackMediaUrls.isEmpty()) {
        m_mediaServiceUrl = m_fallbackMediaUrls.takeFirst();
        appendLog(tr("Trying alternative media URL: %1").arg(m_mediaServiceUrl));
        
        m_onvifClient->setCredentials(m_usernameEdit->text(), m_passwordEdit->text());
        m_onvifClient->getProfiles(m_mediaServiceUrl);
        return;
    }
    
    // Handle 503 Service Not Available - camera might be busy
    if (message.contains("503") || message.contains("Service Not Available") || 
        message.contains("Service Unavailable")) {
        appendLog(tr("Camera is busy. Please wait a moment and try again."));
        appendLog(tr("Tip: Close other connections to this camera if any."));
    }
    
    m_testButton->setEnabled(true);
    m_testButton->setText(tr("Test Connection"));
    m_getProfilesBtn->setEnabled(true);
    m_getProfilesBtn->setText(tr("Get Profiles"));
}

void AddCameraDialog::startPreview(const QString& rtspUrl) {
    stopPreview();
    
    m_previewReceiver = new StreamReceiver(this);
    
    QString username, password;
    CameraType type = static_cast<CameraType>(m_typeCombo->currentData().toInt());
    
    if (type == CameraType::ONVIF) {
        username = m_usernameEdit->text();
        password = m_passwordEdit->text();
    } else {
        username = m_rtspUsernameEdit->text();
        password = m_rtspPasswordEdit->text();
    }
    
    if (m_previewReceiver->open(rtspUrl, username, password)) {
        m_previewWidget->setStreamReceiver(m_previewReceiver);
        m_previewWidget->setCameraName(tr("Preview"));
        m_previewReceiver->start();
        appendLog(tr("Preview started successfully"));
        
        m_testButton->setEnabled(true);
        m_testButton->setText(tr("Test Connection"));
    } else {
        appendLog(tr("Failed to open stream"));
        delete m_previewReceiver;
        m_previewReceiver = nullptr;
        
        m_testButton->setEnabled(true);
        m_testButton->setText(tr("Test Connection"));
    }
}

void AddCameraDialog::stopPreview() {
    if (m_previewReceiver) {
        m_previewWidget->removeStreamReceiver();
        m_previewReceiver->stop();   // 스레드 종료까지 기다린다. GUI 스레드에서 추가로 sleep 하지 않는다.
        delete m_previewReceiver;
        m_previewReceiver = nullptr;
    }
}

void AddCameraDialog::setCredentials(const QString& username, const QString& password) {
    m_usernameEdit->setText(username);
    m_passwordEdit->setText(password);
    m_rtspUsernameEdit->setText(username);
    m_rtspPasswordEdit->setText(password);
}

void AddCameraDialog::startConnectionTest() {
    onTestConnection();
}

void AddCameraDialog::appendLog(const QString& message) {
    m_logEdit->append(QString("[%1] %2")
        .arg(QTime::currentTime().toString("hh:mm:ss"))
        .arg(message));
}

void AddCameraDialog::validate() {
    QString name = m_nameEdit->text().trimmed();
    
    if (name.isEmpty()) {
        QMessageBox::warning(this, tr("Validation Error"), tr("Please enter a camera name."));
        m_nameEdit->setFocus();
        return;
    }
    
    CameraType type = static_cast<CameraType>(m_typeCombo->currentData().toInt());
    
    if (type == CameraType::ONVIF) {
        QString ip = m_ipEdit->text().trimmed();
        if (ip.isEmpty()) {
            QMessageBox::warning(this, tr("Validation Error"), tr("Please enter an IP address."));
            m_ipEdit->setFocus();
            return;
        }
    } else if (type == CameraType::RTSP) {
        QString url = m_rtspMainUrlEdit->text().trimmed();
        if (url.isEmpty()) {
            QMessageBox::warning(this, tr("Validation Error"), tr("Please enter Main RTSP URL."));
            m_rtspMainUrlEdit->setFocus();
            return;
        }
    }
    
    stopPreview();
    accept();
}

CameraInfo AddCameraDialog::getCameraInfo() const {
    CameraInfo info;
    
    info.name = m_nameEdit->text().trimmed();
    info.model = m_modelEdit->text().trimmed();
    info.serialNumber = m_serialNumberEdit->text().trimmed();
    info.manufacturer = m_manufacturerEdit->text().trimmed();
    info.type = static_cast<CameraType>(m_typeCombo->currentData().toInt());
    
    if (info.type == CameraType::ONVIF) {
        info.ip = m_ipEdit->text().trimmed();
        info.port = m_onvifPortSpin->value();
        info.username = m_usernameEdit->text();
        info.password = m_passwordEdit->text();
        info.onvifPath = m_onvifPathEdit->text().trimmed();
        info.rtspUrl = m_mainRtspUrl.isEmpty() ? m_mainRtspUrlDisplay->text() : m_mainRtspUrl;
        info.rtspUrlSub = m_subRtspUrl.isEmpty() ? m_subRtspUrlDisplay->text() : m_subRtspUrl;
    } else if (info.type == CameraType::RTSP) {
        info.rtspUrl = m_rtspMainUrlEdit->text().trimmed();
        info.rtspUrlSub = m_rtspSubUrlEdit->text().trimmed();
        info.username = m_rtspUsernameEdit->text();
        info.password = m_rtspPasswordEdit->text();
        
        QUrl url(info.rtspUrl);
        if (url.isValid()) {
            info.ip = url.host();
            info.port = url.port(554);
        }
    }
    
    return info;
}

void AddCameraDialog::setDeviceInfo(const QString& ip, const QString& name, const QString& serviceUrl, const QString& model) {
    m_nameEdit->setText(name.isEmpty() ? ip : name);
    m_ipEdit->setText(ip);
    m_modelEdit->setText(model);
    
    if (!serviceUrl.isEmpty()) {
        // Parse service URL to extract port and path
        QUrl url(serviceUrl);
        if (url.isValid()) {
            m_onvifPortSpin->setValue(url.port(80));
            QString path = url.path();
            if (!path.isEmpty()) {
                m_onvifPathEdit->setText(path);
            }
        }
        m_typeCombo->setCurrentIndex(m_typeCombo->findData(static_cast<int>(CameraType::ONVIF)));
    }
    
    updateFieldsVisibility();
}

void AddCameraDialog::setCameraInfo(const CameraInfo& info) {
    m_editingCameraId = info.id;
    
    m_nameEdit->setText(info.name);
    m_modelEdit->setText(info.model);
    m_serialNumberEdit->setText(info.serialNumber);
    m_manufacturerEdit->setText(info.manufacturer);
    
    int typeIndex = m_typeCombo->findData(static_cast<int>(info.type));
    if (typeIndex >= 0) {
        m_typeCombo->setCurrentIndex(typeIndex);
    }
    
    if (info.type == CameraType::ONVIF) {
        m_ipEdit->setText(info.ip);
        m_onvifPortSpin->setValue(info.port > 0 ? info.port : 80);
        m_usernameEdit->setText(info.username);
        m_passwordEdit->setText(info.password);
        m_onvifPathEdit->setText(info.onvifPath.isEmpty() ? "/onvif/device_service" : info.onvifPath);
        m_mainRtspUrlDisplay->setText(info.rtspUrl);
        m_subRtspUrlDisplay->setText(info.rtspUrlSub);
        m_mainRtspUrl = info.rtspUrl;
        m_subRtspUrl = info.rtspUrlSub;
    } else if (info.type == CameraType::RTSP) {
        m_rtspMainUrlEdit->setText(info.rtspUrl);
        m_rtspSubUrlEdit->setText(info.rtspUrlSub);
        m_rtspUsernameEdit->setText(info.username);
        m_rtspPasswordEdit->setText(info.password);
    }
    
    updateFieldsVisibility();
    setWindowTitle(tr("Edit Camera"));
}

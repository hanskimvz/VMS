#include "network_settings_dialog.h"
#include "onvif_client.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTime>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

NetworkSettingsDialog::NetworkSettingsDialog(OnvifClient* onvifClient, QWidget* parent)
    : QDialog(parent)
    , m_onvifClient(onvifClient)
{
    setWindowTitle(tr("Network Settings"));
    setMinimumSize(500, 450);
    
    setupUi();
    setupConnections();
}

void NetworkSettingsDialog::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    
    // Device Info
    QGroupBox* deviceGroup = new QGroupBox(tr("Device Information"), this);
    QFormLayout* deviceLayout = new QFormLayout(deviceGroup);
    
    m_deviceIpLabel = new QLabel("-", deviceGroup);
    m_deviceIpLabel->setStyleSheet("font-weight: bold;");
    deviceLayout->addRow(tr("Current IP:"), m_deviceIpLabel);
    
    m_macAddressLabel = new QLabel("-", deviceGroup);
    deviceLayout->addRow(tr("MAC Address:"), m_macAddressLabel);
    
    QHBoxLayout* ifaceLayout = new QHBoxLayout();
    m_interfaceCombo = new QComboBox(deviceGroup);
    m_interfaceCombo->setMinimumWidth(200);
    ifaceLayout->addWidget(m_interfaceCombo);
    
    m_refreshBtn = new QPushButton(tr("Refresh"), deviceGroup);
    m_refreshBtn->setMaximumWidth(80);
    ifaceLayout->addWidget(m_refreshBtn);
    ifaceLayout->addStretch();
    deviceLayout->addRow(tr("Interface:"), ifaceLayout);
    
    mainLayout->addWidget(deviceGroup);
    
    // Network Settings
    QGroupBox* networkGroup = new QGroupBox(tr("IPv4 Settings"), this);
    QVBoxLayout* networkLayout = new QVBoxLayout(networkGroup);
    
    m_dhcpCheck = new QCheckBox(tr("Obtain IP address automatically (DHCP)"), networkGroup);
    networkLayout->addWidget(m_dhcpCheck);
    
    QFormLayout* ipLayout = new QFormLayout();
    ipLayout->setContentsMargins(20, 10, 0, 0);
    
    // IP Address with validation
    m_ipEdit = new QLineEdit(networkGroup);
    m_ipEdit->setPlaceholderText("192.168.1.100");
    QRegularExpression ipRegex("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    m_ipEdit->setValidator(new QRegularExpressionValidator(ipRegex, this));
    ipLayout->addRow(tr("IP Address:"), m_ipEdit);
    
    // Subnet mask
    m_subnetCombo = new QComboBox(networkGroup);
    m_subnetCombo->addItem("255.255.255.0 (/24)", 24);
    m_subnetCombo->addItem("255.255.255.128 (/25)", 25);
    m_subnetCombo->addItem("255.255.255.192 (/26)", 26);
    m_subnetCombo->addItem("255.255.254.0 (/23)", 23);
    m_subnetCombo->addItem("255.255.252.0 (/22)", 22);
    m_subnetCombo->addItem("255.255.248.0 (/21)", 21);
    m_subnetCombo->addItem("255.255.240.0 (/20)", 20);
    m_subnetCombo->addItem("255.255.0.0 (/16)", 16);
    m_subnetCombo->setCurrentIndex(0);
    ipLayout->addRow(tr("Subnet Mask:"), m_subnetCombo);
    
    // Gateway
    m_gatewayEdit = new QLineEdit(networkGroup);
    m_gatewayEdit->setPlaceholderText("192.168.1.1");
    m_gatewayEdit->setValidator(new QRegularExpressionValidator(ipRegex, this));
    ipLayout->addRow(tr("Gateway:"), m_gatewayEdit);
    
    networkLayout->addLayout(ipLayout);
    mainLayout->addWidget(networkGroup);
    
    // Progress and Log
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 0);
    m_progressBar->setVisible(false);
    mainLayout->addWidget(m_progressBar);
    
    QGroupBox* logGroup = new QGroupBox(tr("Log"), this);
    QVBoxLayout* logLayout = new QVBoxLayout(logGroup);
    m_logEdit = new QTextEdit(logGroup);
    m_logEdit->setReadOnly(true);
    m_logEdit->setMaximumHeight(100);
    m_logEdit->setStyleSheet("QTextEdit { background: #1e1e1e; color: #aaa; font-family: monospace; font-size: 10px; }");
    logLayout->addWidget(m_logEdit);
    mainLayout->addWidget(logGroup);
    
    // Warning
    QLabel* warningLabel = new QLabel(
        tr("<b>Warning:</b> Changing network settings may cause connection loss. "
           "Make sure you have physical access to the device."), this);
    warningLabel->setWordWrap(true);
    warningLabel->setStyleSheet("color: #c00; padding: 5px; background: #ffe0e0; border-radius: 3px;");
    mainLayout->addWidget(warningLabel);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_applyBtn = new QPushButton(tr("Apply"), this);
    m_applyBtn->setEnabled(false);
    buttonLayout->addWidget(m_applyBtn);
    
    m_rebootBtn = new QPushButton(tr("Reboot Device"), this);
    m_rebootBtn->setEnabled(false);
    m_rebootBtn->setStyleSheet("QPushButton { background-color: #d9534f; color: white; }");
    buttonLayout->addWidget(m_rebootBtn);
    
    buttonLayout->addStretch();
    
    m_closeBtn = new QPushButton(tr("Close"), this);
    buttonLayout->addWidget(m_closeBtn);
    
    mainLayout->addLayout(buttonLayout);
}

void NetworkSettingsDialog::setupConnections() {
    connect(m_refreshBtn, &QPushButton::clicked, this, &NetworkSettingsDialog::onRefresh);
    connect(m_applyBtn, &QPushButton::clicked, this, &NetworkSettingsDialog::onApply);
    connect(m_rebootBtn, &QPushButton::clicked, this, &NetworkSettingsDialog::onReboot);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_dhcpCheck, &QCheckBox::toggled, this, &NetworkSettingsDialog::onDhcpToggled);
    
    connect(m_interfaceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (index >= 0 && index < m_interfaces.size()) {
            const NetworkInterface& iface = m_interfaces[index];
            m_currentInterfaceToken = iface.token;
            m_macAddressLabel->setText(iface.macAddress);
            
            m_dhcpCheck->setChecked(iface.dhcpEnabled);
            if (!iface.dhcpEnabled) {
                m_ipEdit->setText(iface.ipAddress);
                
                // Find matching prefix in combo
                for (int i = 0; i < m_subnetCombo->count(); ++i) {
                    if (m_subnetCombo->itemData(i).toInt() == iface.prefixLength) {
                        m_subnetCombo->setCurrentIndex(i);
                        break;
                    }
                }
                m_gatewayEdit->setText(iface.gateway);
            }
        }
    });
    
    if (m_onvifClient) {
        connect(m_onvifClient, &OnvifClient::networkInterfacesReceived,
                this, &NetworkSettingsDialog::onNetworkInterfacesReceived);
        connect(m_onvifClient, &OnvifClient::networkSettingsChanged,
                this, &NetworkSettingsDialog::onNetworkSettingsChanged);
        connect(m_onvifClient, &OnvifClient::systemRebooting,
                this, &NetworkSettingsDialog::onSystemRebooting);
        connect(m_onvifClient, &OnvifClient::error,
                this, &NetworkSettingsDialog::onError);
    }
}

void NetworkSettingsDialog::setDeviceInfo(const QString& ip, const QString& serviceUrl,
                                          const QString& username, const QString& password) {
    m_deviceServiceUrl = serviceUrl;
    m_deviceIpLabel->setText(ip);
    
    if (m_onvifClient) {
        m_onvifClient->setCredentials(username, password);
    }
    
    // Auto-refresh
    onRefresh();
}

void NetworkSettingsDialog::onRefresh() {
    if (!m_onvifClient || m_deviceServiceUrl.isEmpty()) {
        appendLog(tr("Error: No device service URL"));
        return;
    }
    
    appendLog(tr("Getting network interfaces..."));
    m_progressBar->setVisible(true);
    m_refreshBtn->setEnabled(false);
    m_applyBtn->setEnabled(false);
    
    m_onvifClient->getNetworkInterfaces(m_deviceServiceUrl);
}

void NetworkSettingsDialog::onApply() {
    if (m_currentInterfaceToken.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("No interface selected."));
        return;
    }
    
    NetworkInterface config;
    config.token = m_currentInterfaceToken;
    config.enabled = true;
    config.dhcpEnabled = m_dhcpCheck->isChecked();
    
    if (!config.dhcpEnabled) {
        config.ipAddress = m_ipEdit->text().trimmed();
        config.prefixLength = m_subnetCombo->currentData().toInt();
        config.gateway = m_gatewayEdit->text().trimmed();
        
        if (config.ipAddress.isEmpty()) {
            QMessageBox::warning(this, tr("Error"), tr("Please enter a valid IP address."));
            m_ipEdit->setFocus();
            return;
        }
    }
    
    QString newIp = config.dhcpEnabled ? "DHCP" : config.ipAddress;
    int ret = QMessageBox::question(this, tr("Confirm Changes"),
        tr("Are you sure you want to change the network settings?\n\n"
           "New IP: %1\n\n"
           "Warning: If the device is on a different subnet after this change, "
           "you may lose connection to it.").arg(newIp),
        QMessageBox::Yes | QMessageBox::No);
    
    if (ret != QMessageBox::Yes) {
        return;
    }
    
    appendLog(tr("Applying network settings..."));
    m_progressBar->setVisible(true);
    setFieldsEnabled(false);
    
    m_gatewayPending = !config.dhcpEnabled && !config.gateway.isEmpty();
    m_pendingRebootNeeded = false;

    m_onvifClient->setNetworkInterfaces(m_deviceServiceUrl, config);

    // Also set gateway if provided and not using DHCP
    if (m_gatewayPending) {
        appendLog(tr("Setting gateway to %1...").arg(config.gateway));
        QTimer::singleShot(1000, this, [this, config]() {
            m_onvifClient->setNetworkDefaultGateway(m_deviceServiceUrl, config.gateway);
        });
    }
}

void NetworkSettingsDialog::onReboot() {
    int ret = QMessageBox::warning(this, tr("Reboot Device"),
        tr("Are you sure you want to reboot the device?\n\n"
           "The device will be unavailable during reboot (typically 30-60 seconds)."),
        QMessageBox::Yes | QMessageBox::No);
    
    if (ret != QMessageBox::Yes) {
        return;
    }
    
    appendLog(tr("Sending reboot command..."));
    m_progressBar->setVisible(true);
    setFieldsEnabled(false);
    
    m_onvifClient->systemReboot(m_deviceServiceUrl);
}

void NetworkSettingsDialog::onDhcpToggled(bool checked) {
    m_ipEdit->setEnabled(!checked);
    m_subnetCombo->setEnabled(!checked);
    m_gatewayEdit->setEnabled(!checked);
    
    if (checked) {
        m_ipEdit->setPlaceholderText(tr("Assigned by DHCP"));
        m_gatewayEdit->setPlaceholderText(tr("Assigned by DHCP"));
    } else {
        m_ipEdit->setPlaceholderText("192.168.1.100");
        m_gatewayEdit->setPlaceholderText("192.168.1.1");
    }
}

void NetworkSettingsDialog::onNetworkInterfacesReceived(const QList<NetworkInterface>& interfaces) {
    m_progressBar->setVisible(false);
    m_refreshBtn->setEnabled(true);
    m_applyBtn->setEnabled(true);
    m_rebootBtn->setEnabled(true);
    
    m_interfaces = interfaces;
    m_interfaceCombo->clear();
    
    if (interfaces.isEmpty()) {
        appendLog(tr("No network interfaces found."));
        return;
    }
    
    appendLog(tr("Found %1 interface(s)").arg(interfaces.size()));
    
    for (const NetworkInterface& iface : interfaces) {
        QString displayName = iface.token;
        if (!iface.name.isEmpty()) {
            displayName = QString("%1 (%2)").arg(iface.name, iface.token);
        }
        if (!iface.ipAddress.isEmpty()) {
            displayName += QString(" - %1").arg(iface.ipAddress);
        }
        m_interfaceCombo->addItem(displayName, iface.token);
        
        appendLog(tr("  %1: %2 (DHCP: %3)")
            .arg(iface.token)
            .arg(iface.ipAddress.isEmpty() ? "No IP" : iface.ipAddress)
            .arg(iface.dhcpEnabled ? "Yes" : "No"));
    }
    
    // Select first interface
    if (!interfaces.isEmpty()) {
        m_interfaceCombo->setCurrentIndex(0);
    }
}

void NetworkSettingsDialog::onNetworkSettingsChanged(bool rebootNeeded) {
    if (m_gatewayPending) {
        // 첫 번째 응답. 게이트웨이 응답이 아직 남았으므로 결과만 기억하고 기다린다.
        m_gatewayPending = false;
        m_pendingRebootNeeded = rebootNeeded;
        return;
    }
    rebootNeeded = rebootNeeded || m_pendingRebootNeeded;
    m_pendingRebootNeeded = false;

    m_progressBar->setVisible(false);
    setFieldsEnabled(true);
    
    if (rebootNeeded) {
        appendLog(tr("Settings applied. Device requires reboot to take effect."));
        
        int ret = QMessageBox::question(this, tr("Reboot Required"),
            tr("Network settings have been applied.\n\n"
               "The device needs to be rebooted for changes to take effect.\n"
               "Do you want to reboot now?"),
            QMessageBox::Yes | QMessageBox::No);
        
        if (ret == QMessageBox::Yes) {
            onReboot();
        }
    } else {
        appendLog(tr("Settings applied successfully."));
        QMessageBox::information(this, tr("Success"),
            tr("Network settings have been applied.\n\n"
               "Note: You may need to reconnect if the IP address changed."));
    }
}

void NetworkSettingsDialog::onSystemRebooting() {
    m_progressBar->setVisible(false);
    appendLog(tr("Device is rebooting..."));
    
    QMessageBox::information(this, tr("Rebooting"),
        tr("The device is now rebooting.\n\n"
           "This typically takes 30-60 seconds. "
           "Please wait and then re-discover the device."));
    
    accept();
}

void NetworkSettingsDialog::onError(const QString& message) {
    m_gatewayPending = false;
    m_pendingRebootNeeded = false;
    m_progressBar->setVisible(false);
    setFieldsEnabled(true);
    m_refreshBtn->setEnabled(true);
    
    appendLog(tr("Error: %1").arg(message));
}

void NetworkSettingsDialog::appendLog(const QString& message) {
    m_logEdit->append(QString("[%1] %2")
        .arg(QTime::currentTime().toString("hh:mm:ss"))
        .arg(message));
}

void NetworkSettingsDialog::setFieldsEnabled(bool enabled) {
    m_interfaceCombo->setEnabled(enabled);
    m_dhcpCheck->setEnabled(enabled);
    m_ipEdit->setEnabled(enabled && !m_dhcpCheck->isChecked());
    m_subnetCombo->setEnabled(enabled && !m_dhcpCheck->isChecked());
    m_gatewayEdit->setEnabled(enabled && !m_dhcpCheck->isChecked());
    m_applyBtn->setEnabled(enabled);
    m_rebootBtn->setEnabled(enabled);
}

QString NetworkSettingsDialog::prefixToSubnetMask(int prefix) const {
    uint32_t mask = prefix ? (0xFFFFFFFF << (32 - prefix)) : 0;
    return QString("%1.%2.%3.%4")
        .arg((mask >> 24) & 0xFF)
        .arg((mask >> 16) & 0xFF)
        .arg((mask >> 8) & 0xFF)
        .arg(mask & 0xFF);
}

int NetworkSettingsDialog::subnetMaskToPrefix(const QString& mask) const {
    QStringList parts = mask.split('.');
    if (parts.size() != 4) return 24;
    
    uint32_t maskValue = (parts[0].toUInt() << 24) |
                         (parts[1].toUInt() << 16) |
                         (parts[2].toUInt() << 8) |
                         parts[3].toUInt();
    
    int prefix = 0;
    while (maskValue & 0x80000000) {
        prefix++;
        maskValue <<= 1;
    }
    return prefix;
}

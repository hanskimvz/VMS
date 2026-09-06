#ifndef NETWORK_SETTINGS_DIALOG_H
#define NETWORK_SETTINGS_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QFormLayout>
#include <QTextEdit>
#include <QProgressBar>

class OnvifClient;
struct NetworkInterface;

class NetworkSettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit NetworkSettingsDialog(OnvifClient* onvifClient, QWidget* parent = nullptr);
    ~NetworkSettingsDialog() = default;

    void setDeviceInfo(const QString& ip, const QString& serviceUrl, 
                       const QString& username = QString(), const QString& password = QString());

private slots:
    void onRefresh();
    void onApply();
    void onReboot();
    void onDhcpToggled(bool checked);
    void onNetworkInterfacesReceived(const QList<NetworkInterface>& interfaces);
    void onNetworkSettingsChanged(bool rebootNeeded);
    void onSystemRebooting();
    void onError(const QString& message);

private:
    void setupUi();
    void setupConnections();
    void appendLog(const QString& message);
    void setFieldsEnabled(bool enabled);
    QString prefixToSubnetMask(int prefix) const;
    int subnetMaskToPrefix(const QString& mask) const;

    OnvifClient* m_onvifClient;
    QString m_deviceServiceUrl;
    QString m_currentInterfaceToken;
    
    // Device info
    QLabel* m_deviceIpLabel = nullptr;
    QLabel* m_macAddressLabel = nullptr;
    
    // Interface selection
    QComboBox* m_interfaceCombo = nullptr;
    QPushButton* m_refreshBtn = nullptr;
    
    // Network settings
    QCheckBox* m_dhcpCheck = nullptr;
    QLineEdit* m_ipEdit = nullptr;
    QComboBox* m_subnetCombo = nullptr;
    QLineEdit* m_gatewayEdit = nullptr;
    
    // Status
    QProgressBar* m_progressBar = nullptr;
    QTextEdit* m_logEdit = nullptr;
    
    // Buttons
    QPushButton* m_applyBtn = nullptr;
    QPushButton* m_rebootBtn = nullptr;
    QPushButton* m_closeBtn = nullptr;
    
    QList<NetworkInterface> m_interfaces;

    // Apply 가 SetNetworkInterfaces 와 SetNetworkDefaultGateway 두 요청을 보내면
    // networkSettingsChanged 도 두 번 온다. 두 번째가 올 때까지 결과 안내를 미룬다.
    bool m_gatewayPending = false;
    bool m_pendingRebootNeeded = false;
};

#endif // NETWORK_SETTINGS_DIALOG_H

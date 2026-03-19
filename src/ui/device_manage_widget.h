#ifndef DEVICE_MANAGE_WIDGET_H
#define DEVICE_MANAGE_WIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QCheckBox>
#include <QMessageBox>
#include <QInputDialog>
#include <QSet>
#include <QList>

#include "device_discovery.h"

class CameraManager;
class OnvifClient;
class DeviceDiscovery;
struct OnvifDevice;
struct CameraInfo;

class DeviceManageWidget : public QWidget {
    Q_OBJECT

public:
    explicit DeviceManageWidget(QWidget* parent = nullptr);
    ~DeviceManageWidget();

    void setCameraManager(CameraManager* manager);
    void setOnvifClient(OnvifClient* client);

public slots:
    void refreshAddedDevices();

protected:
    void showEvent(QShowEvent* event) override;

private slots:
    void onStartSearch();
    void onAddDevice();
    void onModifyIP();
    void onManualAdd();
    
    void onDeleteDevice();
    void onEditDevice();
    
    void onOnvifDeviceDiscovered(const OnvifDevice& device);
    void onOnvifDiscoveryFinished();
    void onOtherDeviceDiscovered(const DiscoveredDevice& device);
    void onOtherDiscoveryFinished();
    void onSearchedDeviceDoubleClicked(int row, int column);
    void onAddedDeviceDoubleClicked(int row, int column);

private:
    void setupUi();
    void setupSearchedDeviceGroup();
    void setupAddedDeviceGroup();
    void applyTableStyle(QTableWidget* table);
    void addDeviceToSearchedTable(const OnvifDevice& device);
    void addDiscoveredDeviceToTable(const DiscoveredDevice& device);
    void updateAddedDeviceTable();
    
    void flushPendingDevices();
    
    CameraManager* m_cameraManager = nullptr;
    OnvifClient* m_onvifClient = nullptr;
    DeviceDiscovery* m_deviceDiscovery = nullptr;
    
    // Discovery state
    QSet<QString> m_onvifDiscoveredIPs;
    QList<DiscoveredDevice> m_pendingUpnpDevices;
    QList<DiscoveredDevice> m_pendingMdnsDevices;
    bool m_onvifDiscoveryFinished = false;
    bool m_otherDiscoveryFinished = false;
    
    // Searched device section
    QGroupBox* m_searchedGroup = nullptr;
    QTableWidget* m_searchedTable = nullptr;
    QPushButton* m_startSearchBtn = nullptr;
    QPushButton* m_addDeviceBtn = nullptr;
    QPushButton* m_modifyIPBtn = nullptr;
    QPushButton* m_manualAddBtn = nullptr;
    
    // Added device section
    QGroupBox* m_addedGroup = nullptr;
    QTableWidget* m_addedTable = nullptr;
    QPushButton* m_deleteBtn = nullptr;
    QPushButton* m_editBtn = nullptr;
};

#endif // DEVICE_MANAGE_WIDGET_H

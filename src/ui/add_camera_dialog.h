#ifndef ADD_CAMERA_DIALOG_H
#define ADD_CAMERA_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QFormLayout>
#include <QTextEdit>
#include "camera.h"

class OnvifClient;
class StreamReceiver;
class VideoWidget;
struct OnvifCapabilities;
struct OnvifDeviceInfo;
struct OnvifProfile;

class AddCameraDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit AddCameraDialog(OnvifClient* onvifClient, QWidget* parent = nullptr);
    ~AddCameraDialog();
    
    CameraInfo getCameraInfo() const;
    QString getCameraId() const { return m_editingCameraId; }
    bool isEditMode() const { return !m_editingCameraId.isEmpty(); }
    
    void setDeviceInfo(const QString& ip, const QString& name, const QString& serviceUrl = QString(), const QString& model = QString());
    void setCameraInfo(const CameraInfo& info);
    
private slots:
    void onTypeChanged(int index);
    void onTestConnection();
    void onGetProfiles();
    void onMainProfileChanged(int index);
    void onSubProfileChanged(int index);
    void onCapabilitiesReceived(const OnvifCapabilities& capabilities);
    void onDeviceInformationReceived(const OnvifDeviceInfo& info);
    void onProfilesReceived(const QList<OnvifProfile>& profiles);
    void onStreamUriReceived(const QString& profileToken, const QString& uri);
    void onOnvifError(const QString& message);
    void validate();
    
private:
    void setupUi();
    void setupConnections();
    void updateFieldsVisibility();
    void stopPreview();
    void startPreview(const QString& rtspUrl);
    void appendLog(const QString& message);
    
    OnvifClient* m_onvifClient;
    StreamReceiver* m_previewReceiver = nullptr;
    
    // Basic info
    QLineEdit* m_nameEdit = nullptr;
    QLineEdit* m_modelEdit = nullptr;
    QLineEdit* m_serialNumberEdit = nullptr;
    QLineEdit* m_manufacturerEdit = nullptr;
    QComboBox* m_typeCombo = nullptr;
    
    // ONVIF fields
    QGroupBox* m_onvifGroup = nullptr;
    QLineEdit* m_ipEdit = nullptr;
    QSpinBox* m_onvifPortSpin = nullptr;
    QLineEdit* m_usernameEdit = nullptr;
    QLineEdit* m_passwordEdit = nullptr;
    QLineEdit* m_onvifPathEdit = nullptr;
    
    // Profile selection
    QGroupBox* m_profileGroup = nullptr;
    QComboBox* m_mainProfileCombo = nullptr;
    QComboBox* m_subProfileCombo = nullptr;
    QPushButton* m_getProfilesBtn = nullptr;
    QLabel* m_mainProfileInfoLabel = nullptr;
    QLabel* m_subProfileInfoLabel = nullptr;
    QLineEdit* m_mainRtspUrlDisplay = nullptr;
    QLineEdit* m_subRtspUrlDisplay = nullptr;
    
    // Preview
    QGroupBox* m_previewGroup = nullptr;
    VideoWidget* m_previewWidget = nullptr;
    
    // Log
    QTextEdit* m_logEdit = nullptr;
    
    // RTSP fields
    QGroupBox* m_rtspGroup = nullptr;
    QLineEdit* m_rtspMainUrlEdit = nullptr;
    QLineEdit* m_rtspSubUrlEdit = nullptr;
    QLineEdit* m_rtspUsernameEdit = nullptr;
    QLineEdit* m_rtspPasswordEdit = nullptr;
    
    // Buttons
    QPushButton* m_testButton = nullptr;
    QPushButton* m_okButton = nullptr;
    QPushButton* m_cancelButton = nullptr;
    
    QString m_editingCameraId;
    QString m_deviceServiceUrl;
    QString m_mediaServiceUrl;
    QString m_mainProfileToken;
    QString m_subProfileToken;
    QString m_mainRtspUrl;
    QString m_subRtspUrl;
    QList<OnvifProfile> m_profiles;
    QStringList m_fallbackMediaUrls;
    bool m_useGetServices = true;
    int m_pendingStreamUriRequests = 0;
};

#endif // ADD_CAMERA_DIALOG_H

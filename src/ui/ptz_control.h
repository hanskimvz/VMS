#ifndef PTZ_CONTROL_H
#define PTZ_CONTROL_H

#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include "onvif_client.h"

class PtzControl : public QWidget {
    Q_OBJECT
    
public:
    explicit PtzControl(QWidget* parent = nullptr);
    ~PtzControl();
    
signals:
    void ptzCommand(PtzAction action, float speed);
    
private slots:
    void onButtonPressed();
    void onButtonReleased();
    void onSpeedChanged(int value);
    
private:
    void setupUi();
    QPushButton* createPtzButton(const QString& text, PtzAction action);
    
    float m_speed = 0.5f;
    QSlider* m_speedSlider = nullptr;
    
    QPushButton* m_upButton = nullptr;
    QPushButton* m_downButton = nullptr;
    QPushButton* m_leftButton = nullptr;
    QPushButton* m_rightButton = nullptr;
    QPushButton* m_upLeftButton = nullptr;
    QPushButton* m_upRightButton = nullptr;
    QPushButton* m_downLeftButton = nullptr;
    QPushButton* m_downRightButton = nullptr;
    QPushButton* m_zoomInButton = nullptr;
    QPushButton* m_zoomOutButton = nullptr;
};

#endif // PTZ_CONTROL_H

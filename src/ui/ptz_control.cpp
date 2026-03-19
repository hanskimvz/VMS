#include "ptz_control.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>

PtzControl::PtzControl(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

PtzControl::~PtzControl() {
}

void PtzControl::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    
    QGroupBox* directionGroup = new QGroupBox(tr("Direction"), this);
    QGridLayout* directionLayout = new QGridLayout(directionGroup);
    directionLayout->setSpacing(2);
    
    m_upLeftButton = createPtzButton("↖", PtzAction::UpLeft);
    m_upButton = createPtzButton("↑", PtzAction::Up);
    m_upRightButton = createPtzButton("↗", PtzAction::UpRight);
    m_leftButton = createPtzButton("←", PtzAction::Left);
    m_rightButton = createPtzButton("→", PtzAction::Right);
    m_downLeftButton = createPtzButton("↙", PtzAction::DownLeft);
    m_downButton = createPtzButton("↓", PtzAction::Down);
    m_downRightButton = createPtzButton("↘", PtzAction::DownRight);
    
    directionLayout->addWidget(m_upLeftButton, 0, 0);
    directionLayout->addWidget(m_upButton, 0, 1);
    directionLayout->addWidget(m_upRightButton, 0, 2);
    directionLayout->addWidget(m_leftButton, 1, 0);
    directionLayout->addWidget(new QWidget(), 1, 1);
    directionLayout->addWidget(m_rightButton, 1, 2);
    directionLayout->addWidget(m_downLeftButton, 2, 0);
    directionLayout->addWidget(m_downButton, 2, 1);
    directionLayout->addWidget(m_downRightButton, 2, 2);
    
    mainLayout->addWidget(directionGroup);
    
    QGroupBox* zoomGroup = new QGroupBox(tr("Zoom"), this);
    QHBoxLayout* zoomLayout = new QHBoxLayout(zoomGroup);
    
    m_zoomOutButton = createPtzButton("-", PtzAction::ZoomOut);
    m_zoomInButton = createPtzButton("+", PtzAction::ZoomIn);
    
    zoomLayout->addWidget(m_zoomOutButton);
    zoomLayout->addWidget(m_zoomInButton);
    
    mainLayout->addWidget(zoomGroup);
    
    QGroupBox* speedGroup = new QGroupBox(tr("Speed"), this);
    QVBoxLayout* speedLayout = new QVBoxLayout(speedGroup);
    
    m_speedSlider = new QSlider(Qt::Horizontal, this);
    m_speedSlider->setRange(1, 10);
    m_speedSlider->setValue(5);
    m_speedSlider->setTickPosition(QSlider::TicksBelow);
    m_speedSlider->setTickInterval(1);
    
    connect(m_speedSlider, &QSlider::valueChanged, this, &PtzControl::onSpeedChanged);
    
    QHBoxLayout* speedLabelLayout = new QHBoxLayout();
    speedLabelLayout->addWidget(new QLabel(tr("Slow")));
    speedLabelLayout->addStretch();
    speedLabelLayout->addWidget(new QLabel(tr("Fast")));
    
    speedLayout->addWidget(m_speedSlider);
    speedLayout->addLayout(speedLabelLayout);
    
    mainLayout->addWidget(speedGroup);
    mainLayout->addStretch();
}

QPushButton* PtzControl::createPtzButton(const QString& text, PtzAction action) {
    QPushButton* button = new QPushButton(text, this);
    button->setFixedSize(50, 50);
    button->setProperty("ptzAction", static_cast<int>(action));
    
    button->setStyleSheet(
        "QPushButton {"
        "  background-color: #3e3e42;"
        "  border: 1px solid #555;"
        "  border-radius: 5px;"
        "  font-size: 18px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #505050;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #007acc;"
        "}"
    );
    
    connect(button, &QPushButton::pressed, this, &PtzControl::onButtonPressed);
    connect(button, &QPushButton::released, this, &PtzControl::onButtonReleased);
    
    return button;
}

void PtzControl::onButtonPressed() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    
    PtzAction action = static_cast<PtzAction>(button->property("ptzAction").toInt());
    emit ptzCommand(action, m_speed);
}

void PtzControl::onButtonReleased() {
    emit ptzCommand(PtzAction::Stop, 0);
}

void PtzControl::onSpeedChanged(int value) {
    m_speed = value / 10.0f;
}

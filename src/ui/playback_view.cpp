#include "playback_view.h"
#include "video_widget.h"
#include "timeline_widget.h"
#include "playback_controller.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QSplitter>
#include <QFileDialog>

PlaybackView::PlaybackView(QWidget* parent)
    : QWidget(parent)
    , m_controller(std::make_unique<PlaybackController>(this))
{
    setupUi();
    setupConnections();
}

PlaybackView::~PlaybackView() {
}

void PlaybackView::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(4);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    
    QSplitter* splitter = new QSplitter(Qt::Vertical, this);
    
    m_videoWidget = new VideoWidget(splitter);
    splitter->addWidget(m_videoWidget);
    
    QWidget* controlsContainer = new QWidget(splitter);
    QVBoxLayout* controlsLayout = new QVBoxLayout(controlsContainer);
    controlsLayout->setSpacing(4);
    controlsLayout->setContentsMargins(0, 0, 0, 0);
    
    m_timeline = new TimelineWidget(controlsContainer);
    m_timeline->setMinimumHeight(80);
    controlsLayout->addWidget(m_timeline);
    
    m_seekSlider = new QSlider(Qt::Horizontal, controlsContainer);
    m_seekSlider->setRange(0, 1000);
    controlsLayout->addWidget(m_seekSlider);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);
    
    m_playButton = new QPushButton(tr("Play"), controlsContainer);
    m_playButton->setFixedWidth(80);
    buttonLayout->addWidget(m_playButton);
    
    m_stopButton = new QPushButton(tr("Stop"), controlsContainer);
    m_stopButton->setFixedWidth(80);
    buttonLayout->addWidget(m_stopButton);
    
    buttonLayout->addSpacing(20);
    
    m_timeLabel = new QLabel("00:00:00", controlsContainer);
    m_timeLabel->setStyleSheet("font-family: monospace; font-size: 12px;");
    buttonLayout->addWidget(m_timeLabel);
    
    buttonLayout->addWidget(new QLabel("/", controlsContainer));
    
    m_durationLabel = new QLabel("00:00:00", controlsContainer);
    m_durationLabel->setStyleSheet("font-family: monospace; font-size: 12px;");
    buttonLayout->addWidget(m_durationLabel);
    
    buttonLayout->addStretch();
    
    buttonLayout->addWidget(new QLabel(tr("Speed:"), controlsContainer));
    m_speedCombo = new QComboBox(controlsContainer);
    m_speedCombo->addItem("0.25x", 0.25);
    m_speedCombo->addItem("0.5x", 0.5);
    m_speedCombo->addItem("1x", 1.0);
    m_speedCombo->addItem("2x", 2.0);
    m_speedCombo->addItem("4x", 4.0);
    m_speedCombo->setCurrentIndex(2);
    buttonLayout->addWidget(m_speedCombo);
    
    buttonLayout->addSpacing(20);
    
    QPushButton* openButton = new QPushButton(tr("Open File..."), controlsContainer);
    connect(openButton, &QPushButton::clicked, [this]() {
        QString file = QFileDialog::getOpenFileName(this, tr("Open Recording"),
            QString(), tr("Video Files (*.mp4 *.mkv *.avi *.ts)"));
        if (!file.isEmpty()) {
            loadRecording(file);
        }
    });
    buttonLayout->addWidget(openButton);
    
    controlsLayout->addLayout(buttonLayout);
    
    splitter->addWidget(controlsContainer);
    splitter->setSizes({700, 150});
    
    mainLayout->addWidget(splitter);
}

void PlaybackView::setupConnections() {
    connect(m_playButton, &QPushButton::clicked, [this]() {
        if (m_controller->state() == PlaybackController::Playing) {
            onPause();
        } else {
            onPlay();
        }
    });
    
    connect(m_stopButton, &QPushButton::clicked, this, &PlaybackView::onStop);
    
    connect(m_speedCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PlaybackView::onSpeedChanged);
    
    connect(m_seekSlider, &QSlider::sliderPressed, [this]() {
        if (m_controller->state() == PlaybackController::Playing) {
            m_controller->pause();
        }
    });
    
    connect(m_seekSlider, &QSlider::sliderReleased, [this]() {
        int64_t duration = m_controller->duration();
        int64_t position = static_cast<int64_t>(m_seekSlider->value() / 1000.0 * duration);
        m_controller->seek(position);
        m_controller->play();
    });
    
    connect(m_controller.get(), &PlaybackController::positionChanged,
            this, &PlaybackView::onPositionChanged);
    
    connect(m_controller.get(), &PlaybackController::stateChanged,
            this, &PlaybackView::onStateChanged);
    
    connect(m_controller.get(), &PlaybackController::frameReady,
            [this](const QImage& frame) {
                m_videoWidget->displayFrame(frame);
            });
    
    connect(m_timeline, &TimelineWidget::positionClicked,
            this, &PlaybackView::onTimelineClicked);
}

void PlaybackView::setCameraId(const QString& cameraId) {
    m_currentCameraId = cameraId;
}

void PlaybackView::loadRecording(const QString& filePath) {
    m_currentFile = filePath;
    
    if (m_controller->openFile(filePath)) {
        int64_t duration = m_controller->duration();
        m_durationLabel->setText(QString("%1:%2:%3")
            .arg(duration / 3600000, 2, 10, QChar('0'))
            .arg((duration / 60000) % 60, 2, 10, QChar('0'))
            .arg((duration / 1000) % 60, 2, 10, QChar('0')));
        
        m_timeline->setDuration(duration);
        m_controller->play();
    }
}

void PlaybackView::onPlay() {
    m_controller->play();
    updatePlayButton();
}

void PlaybackView::onPause() {
    m_controller->pause();
    updatePlayButton();
}

void PlaybackView::onStop() {
    m_controller->stop();
    updatePlayButton();
}

void PlaybackView::onSeek(int64_t timestamp) {
    m_controller->seek(timestamp);
}

void PlaybackView::onSpeedChanged(int index) {
    double speed = m_speedCombo->itemData(index).toDouble();
    m_controller->setSpeed(speed);
}

void PlaybackView::onPositionChanged(int64_t position) {
    updateTimeDisplay(position);
    
    int64_t duration = m_controller->duration();
    if (duration > 0) {
        int sliderPos = static_cast<int>(position * 1000.0 / duration);
        m_seekSlider->blockSignals(true);
        m_seekSlider->setValue(sliderPos);
        m_seekSlider->blockSignals(false);
    }
    
    m_timeline->setCurrentPosition(position);
}

void PlaybackView::onStateChanged(int state) {
    Q_UNUSED(state);
    updatePlayButton();
}

void PlaybackView::onTimelineClicked(int64_t timestamp) {
    m_controller->seek(timestamp);
}

void PlaybackView::updatePlayButton() {
    if (m_controller->state() == PlaybackController::Playing) {
        m_playButton->setText(tr("Pause"));
    } else {
        m_playButton->setText(tr("Play"));
    }
}

void PlaybackView::updateTimeDisplay(int64_t position) {
    m_timeLabel->setText(QString("%1:%2:%3")
        .arg(position / 3600000, 2, 10, QChar('0'))
        .arg((position / 60000) % 60, 2, 10, QChar('0'))
        .arg((position / 1000) % 60, 2, 10, QChar('0')));
}

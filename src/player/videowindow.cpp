#include "videowindow.h"
#include <QStyle>
#include <QMessageBox>
#include <QDebug>

VideoWindow::VideoWindow(QWidget *parent) : QWidget(parent),
    m_player(nullptr),
    m_videoWidget(nullptr)
{
    setWindowTitle("video player");
    setMinimumSize(800, 600);
    
    // 创建媒体播放器
    m_player = new QMediaPlayer(this);
    m_videoWidget = new QVideoWidget(this);
    m_player->setVideoOutput(m_videoWidget);
    
    setupUI();
    setupConnections();
}

VideoWindow::~VideoWindow()
{
    // Qt 的对象树会自动管理内存
}

void VideoWindow::setVideoList(const std::vector<TheButtonInfo>& videos)
{
    m_videos = videos;
    
    if (!m_videos.empty()) {
        TheButtonInfo* firstVideo = const_cast<TheButtonInfo*>(&m_videos.at(0));
        if (firstVideo && firstVideo->url) {
            m_player->setMedia(*firstVideo->url);
            m_player->play();
        }
    }
}

void VideoWindow::setupUI()
{
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // 视频区域
    m_videoWidget->setMinimumHeight(400);
    mainLayout->addWidget(m_videoWidget, 1);
    
    // 控制面板
    QWidget *controlWidget = new QWidget(this);
    QVBoxLayout *controlLayout = new QVBoxLayout(controlWidget);
    
    // 播放控制行
    QWidget *playbackRow = new QWidget();
    QHBoxLayout *playbackLayout = new QHBoxLayout(playbackRow);
    
    m_playPauseButton = new QToolButton();
    m_playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    m_playPauseButton->setFixedSize(40, 40);
    m_playPauseButton->setToolTip("Play/Pause");
    
    m_progressSlider = new QSlider(Qt::Horizontal);
    m_progressSlider->setRange(0, 100);
    m_progressSlider->setMinimumHeight(25);
    
    m_timeLabel = new QLabel("00:00 / 00:00");
    m_timeLabel->setFixedWidth(120);
    m_timeLabel->setAlignment(Qt::AlignCenter);
    
    playbackLayout->addWidget(m_playPauseButton);
    playbackLayout->addWidget(new QLabel("progress:"));
    playbackLayout->addWidget(m_progressSlider, 1);
    playbackLayout->addWidget(m_timeLabel);
    
    // 设置控制行
    QWidget *settingsRow = new QWidget();
    QHBoxLayout *settingsLayout = new QHBoxLayout(settingsRow);
    
    settingsLayout->addWidget(new QLabel("volume:"));
    m_volumeSlider = new QSlider(Qt::Horizontal);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(50);
    m_volumeSlider->setFixedWidth(100);
    settingsLayout->addWidget(m_volumeSlider);
    
    settingsLayout->addSpacing(20);
    settingsLayout->addWidget(new QLabel("speed:"));
    m_speedComboBox = new QComboBox();
    m_speedComboBox->addItem("0.5x", 0.5);
    m_speedComboBox->addItem("normal", 1.0);
    m_speedComboBox->addItem("1.25x", 1.25);
    m_speedComboBox->addItem("1.5x", 1.5);
    m_speedComboBox->addItem("2.0x", 2.0);
    m_speedComboBox->setCurrentIndex(1);
    m_speedComboBox->setFixedWidth(100);
    settingsLayout->addWidget(m_speedComboBox);
    
    settingsLayout->addStretch(1);
    
    // 添加到控制面板
    controlLayout->addWidget(playbackRow);
    controlLayout->addWidget(settingsRow);
    
    // 添加到主布局
    mainLayout->addWidget(controlWidget);
}

void VideoWindow::setupConnections()
{
    // 播放/暂停按钮
    connect(m_playPauseButton, &QToolButton::clicked, this, &VideoWindow::togglePlayPause);
    
    // 播放状态改变
    connect(m_player, &QMediaPlayer::stateChanged, this, &VideoWindow::updatePlayPauseButton);
    
    // 播放进度
    connect(m_player, &QMediaPlayer::positionChanged, this, &VideoWindow::updatePosition);
    connect(m_player, &QMediaPlayer::durationChanged, this, &VideoWindow::updateDuration);
    
    // 进度条控制
    connect(m_progressSlider, &QSlider::sliderMoved, this, &VideoWindow::setPosition);
    
    // 音量控制
    connect(m_volumeSlider, &QSlider::valueChanged, this, &VideoWindow::setVolume);
    
    // 倍速控制
    connect(m_speedComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &VideoWindow::setPlaybackRate);
}

void VideoWindow::togglePlayPause()
{
    if (m_player->state() == QMediaPlayer::PlayingState) {
        m_player->pause();
    } else {
        m_player->play();
    }
}

void VideoWindow::updatePlayPauseButton()
{
    if (m_player->state() == QMediaPlayer::PlayingState) {
        m_playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    } else {
        m_playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    }
}

void VideoWindow::updatePosition(qint64 position)
{
    if (!m_progressSlider->isSliderDown() && m_player->duration() > 0) {
        m_progressSlider->setValue(static_cast<int>((position * 100) / m_player->duration()));
    }
    
    QString currentTime = formatTime(position);
    QString totalTime = formatTime(m_player->duration());
    m_timeLabel->setText(QString("%1 / %2").arg(currentTime).arg(totalTime));
}

void VideoWindow::updateDuration(qint64 duration)
{
    Q_UNUSED(duration)
    m_progressSlider->setRange(0, 100);
    updatePosition(m_player->position());
}

void VideoWindow::setPosition(int position)
{
    if (m_player->duration() > 0) {
        qint64 newPosition = static_cast<qint64>((position * m_player->duration()) / 100);
        m_player->setPosition(newPosition);
    }
}

void VideoWindow::setVolume(int volume)
{
    m_player->setVolume(volume);
}

void VideoWindow::setPlaybackRate(int index)
{
    qreal rate = m_speedComboBox->itemData(index).toDouble();
    
    // 直接设置播放速率
    m_player->setPlaybackRate(rate);
    
    qDebug() << "Playback rate set to:" << rate;
    
    // 可选：延迟检查实际播放速率（如果需要）
    QTimer::singleShot(100, [this, rate]() {
        qreal actualRate = m_player->playbackRate();
        qDebug() << "Actual playback rate:" << actualRate;
        
        // 如果设置不成功，可以给用户提示
        if (!qFuzzyCompare(actualRate, rate)) {
            qDebug() << "Note: Playback rate may be limited by system capabilities";
        }
    });
}

QString VideoWindow::formatTime(qint64 milliseconds)
{
    qint64 seconds = milliseconds / 1000;
    qint64 minutes = seconds / 60;
    seconds = seconds % 60;
    
    qint64 hours = minutes / 60;
    minutes = minutes % 60;
    
    if (hours > 0) {
        return QString("%1:%2:%3")
            .arg(hours, 2, 10, QChar('0'))
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
    } else {
        return QString("%1:%2")
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
    }
}

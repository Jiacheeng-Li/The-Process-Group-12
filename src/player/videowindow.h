#ifndef VIDEOWINDOW_H
#define VIDEOWINDOW_H

#include <QWidget>
#include <QVideoWidget>
#include <QMediaPlayer>
#include <QSlider>
#include <QLabel>
#include <QToolButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStyle>
#include <QTimer>
#include <vector>
#include "the_button.h"

class VideoWindow : public QWidget
{
    Q_OBJECT

public:
    explicit VideoWindow(QWidget *parent = nullptr);
    ~VideoWindow();

    // 设置视频列表
    void setVideoList(const std::vector<TheButtonInfo>& videos);

private slots:
    void togglePlayPause();
    void updatePlayPauseButton();
    void updatePosition(qint64 position);
    void updateDuration(qint64 duration);
    void setPosition(int position);
    void setVolume(int volume);
    void setPlaybackRate(int index);

private:
    void setupUI();
    void setupConnections();
    QString formatTime(qint64 milliseconds);

    QMediaPlayer *m_player;
    QVideoWidget *m_videoWidget;
    
    // 控制控件
    QToolButton *m_playPauseButton;
    QSlider *m_progressSlider;
    QSlider *m_volumeSlider;
    QComboBox *m_speedComboBox;
    QLabel *m_timeLabel;

    // 视频列表
    std::vector<TheButtonInfo> m_videos;
};

#endif // VIDEOWINDOW_H

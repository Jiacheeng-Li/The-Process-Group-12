#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QWidget>
#include <QMediaPlayer>
#include <QVideoWidget>

class VideoPlayer : public QWidget
{
    Q_OBJECT
public:
    explicit VideoPlayer(QWidget *parent = nullptr);

private:
    QMediaPlayer *mediaPlayer;
    QVideoWidget *videoWidget;
    
    // TODO: Add playback controls
    // TODO: Add interaction handlers (like, comment, share)
};

#endif // VIDEOPLAYER_H

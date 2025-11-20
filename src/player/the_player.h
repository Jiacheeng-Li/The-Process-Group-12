#ifndef CW2_THE_PLAYER_H
#define CW2_THE_PLAYER_H

#include <QApplication>
#include <QMediaPlayer>
#include "the_button.h"
#include <vector>
#include <QTimer>

class ThePlayer : public QMediaPlayer {
    Q_OBJECT

private:
    std::vector<TheButtonInfo>* infos;
    std::vector<TheButton*>* buttons;
    QTimer* mTimer;
    long updateCount = 0;

public:
    ThePlayer(QObject *parent = nullptr) : QMediaPlayer(parent) {
        setVolume(50);
        connect(this, SIGNAL(stateChanged(QMediaPlayer::State)), 
                this, SLOT(playStateChanged(QMediaPlayer::State)));
    }

    // all buttons have been setup, store pointers here
    void setContent(std::vector<TheButton*>* b, std::vector<TheButtonInfo>* i);

public slots:
    // 添加倍速控制 - 简化版本
    void setPlaybackRate(qreal rate) {
        qDebug() << "Setting playback rate to:" << rate;
        QMediaPlayer::setPlaybackRate(rate);
        
        // 检查实际设置的倍速
        QTimer::singleShot(100, [this, rate]() {
            qreal actualRate = this->playbackRate();
            qDebug() << "Actual playback rate:" << actualRate;
            if (!qFuzzyCompare(actualRate, rate)) {
                qDebug() << "Warning: Playback rate may not be supported";
            }
        });
    }

private slots:
    void shuffle();
    void playStateChanged(QMediaPlayer::State ms);

public slots:
    void jumpTo(TheButtonInfo* button);
};

#endif // CW2_THE_PLAYER_H
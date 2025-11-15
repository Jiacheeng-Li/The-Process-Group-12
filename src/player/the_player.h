//
//  The Player - Enhanced for Mobile
//

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
    int currentIndex = 0;

public:
    ThePlayer() : QMediaPlayer(NULL) {
        setVolume(50); // 设置默认音量为50%
        connect (this, SIGNAL (stateChanged(QMediaPlayer::State)), 
                this, SLOT (playStateChanged(QMediaPlayer::State)) );

        // 关闭自动切换功能，改为用户手动切换
        mTimer = new QTimer(NULL);
        mTimer->setInterval(1000); // 1000ms is one second
        // 不自动启动 timer，移除自动shuffle功能
        // mTimer->start();
        // connect( mTimer, SIGNAL (timeout()), SLOT ( shuffle() ) );
    }

    // all buttons have been setup, store pointers here
    void setContent(std::vector<TheButton*>* b, std::vector<TheButtonInfo>* i);
    
    // 获取当前视频索引
    int getCurrentIndex() const { return currentIndex; }
    
    // 播放指定索引的视频
    void playVideoAtIndex(int index);
    
    // 播放下一个视频
    void playNext();
    
    // 播放上一个视频
    void playPrevious();

private slots:

    // change the image and video for one button every one second
    // 这个功能现在不再使用
    void shuffle();

    void playStateChanged (QMediaPlayer::State ms);

public slots:

    // start playing this ButtonInfo
    void jumpTo (TheButtonInfo* button);
};

#endif //CW2_THE_PLAYER_H

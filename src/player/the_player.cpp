//
//  The Player - Enhanced for Mobile
//

#include "the_player.h"

// all buttons have been setup, store pointers here
void ThePlayer::setContent(std::vector<TheButton*>* b, std::vector<TheButtonInfo>* i) {
    buttons = b;
    infos = i;
    currentIndex = 0;
    if (buttons->size() > 0) {
        jumpTo(buttons->at(0)->info);
    }
}

// change the image and video for one button every one second
// 这个功能现在不再使用，但保留代码以兼容
void ThePlayer::shuffle() {
    TheButtonInfo* i = & infos -> at (rand() % infos->size() );
    buttons -> at( updateCount++ % buttons->size() ) -> init( i );
}

void ThePlayer::playStateChanged (QMediaPlayer::State ms) {
    switch (ms) {
        case QMediaPlayer::State::StoppedState:
            // 视频播放结束后自动播放下一个
            playNext();
            break;
    default:
        break;
    }
}

void ThePlayer::jumpTo (TheButtonInfo* button) {
    setMedia( * button -> url);
    play();
    
    // 更新当前索引
    for (size_t i = 0; i < infos->size(); i++) {
        if (&infos->at(i) == button) {
            currentIndex = i;
            break;
        }
    }
}

void ThePlayer::playVideoAtIndex(int index) {
    if (index >= 0 && index < (int)infos->size()) {
        currentIndex = index;
        jumpTo(&infos->at(index));
    }
}

void ThePlayer::playNext() {
    if (infos->size() > 0) {
        currentIndex = (currentIndex + 1) % infos->size();
        jumpTo(&infos->at(currentIndex));
    }
}

void ThePlayer::playPrevious() {
    if (infos->size() > 0) {
        currentIndex = (currentIndex - 1 + infos->size()) % infos->size();
        jumpTo(&infos->at(currentIndex));
    }
}

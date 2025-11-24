//
//
//

#include "the_player.h"

// all buttons have been setup, store pointers here
void ThePlayer::setContent(std::vector<TheButton*>* b, std::vector<TheButtonInfo>* i) {
    buttons = b;
    infos = i;
    if (buttons && !buttons->empty() && buttons->at(0) && buttons->at(0)->info) {
        jumpTo(buttons->at(0)->info);
    } else if (infos && !infos->empty() && infos->at(0).url) {
        setMedia(*infos->at(0).url);
        play();
    }
}

// change the image and video for one button every one second
void ThePlayer::shuffle() {
    if (!buttons || buttons->empty() || !infos || infos->empty()) {
        return;
    }
    TheButtonInfo* i = & infos -> at (rand() % infos->size() );
//        setMedia(*i->url);
    buttons -> at( updateCount++ % buttons->size() ) -> init( i );
}

void ThePlayer::playStateChanged (QMediaPlayer::State ms) {
    switch (ms) {
        case QMediaPlayer::State::StoppedState:
            play(); // starting playing again...
            break;
    default:
        break;
    }
}

void ThePlayer::jumpTo (TheButtonInfo* button) {
    setMedia( * button -> url);
    play();
}

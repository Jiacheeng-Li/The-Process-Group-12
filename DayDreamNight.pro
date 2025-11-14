QT += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TARGET = DayDreamNight
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    home/homepage.cpp \
    player/videoplayer.cpp \
    friends/friendspage.cpp \
    capture/recordpage.cpp \
    chat/chatpage.cpp \
    profile/profilepage.cpp \
    shared/sharedcomponents.cpp

HEADERS += \
    mainwindow.h \
    home/homepage.h \
    player/videoplayer.h \
    friends/friendspage.h \
    capture/recordpage.h \
    chat/chatpage.h \
    profile/profilepage.h \
    shared/sharedcomponents.h

# Default rules for deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

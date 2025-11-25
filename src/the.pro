QT += core gui widgets multimedia multimediawidgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# 主函数
SOURCES += \
    home/tomeo.cpp

# Capture 文件夹（录制和发布）
SOURCES += \
    capture/recordpage.cpp \
    capture/publishpage.cpp \
    capture/popuppanel.cpp

# Friends 文件夹（朋友圈）
SOURCES += \
    friends/friendspage.cpp \
    friends/frienditem.cpp \
    friends/video_viewer.cpp

# Home 文件夹
# (主函数已在上面添加)

# Player 文件夹（播放器组件）
SOURCES += \
    player/the_player.cpp \
    player/the_button.cpp

# Profile 文件夹（个人主页）
SOURCES += \
    profile/profile_page.cpp

# Chat 文件夹（聊天页面）
SOURCES += \
    chat/chat_page.cpp

# 头文件
HEADERS += \
    capture/recordpage.h \
    capture/publishpage.h \
    capture/popuppanel.h \
    friends/friendspage.h \
    friends/frienditem.h \
    friends/video_viewer.h \
    player/the_player.h \
    player/the_button.h \
    profile/profile_page.h \
    chat/chat_page.h

# 包含路径设置（相对于 src 目录）
INCLUDEPATH += \
    . \
    home \
    capture \
    friends \
    player \
    profile \
    chat

# 资源文件
RESOURCES += \
    capture/resources.qrc \
    home/home.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

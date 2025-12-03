QT += core gui widgets multimedia multimediawidgets texttospeech

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Main entry point source
SOURCES += \
    home/Antipode.cpp

# Explicitly tell MOC to process Antipode.cpp (contains Q_OBJECT classes)
# MOC will generate moc_Antipode.cpp automatically

# Capture module sources (record & publish flow)
SOURCES += \
    capture/recordpage.cpp \
    capture/publishpage.cpp \
    capture/popuppanel.cpp

# Friends module sources (social feed)
SOURCES += \
    friends/friendspage.cpp \
    friends/frienditem.cpp \
    friends/video_viewer.cpp

# Home module (main window already included above)

# Player components (shared playback controls)
SOURCES += \
    player/the_player.cpp \
    player/the_button.cpp

# Profile module (user profile page)
SOURCES += \
    profile/profile_page.cpp

# Chat module (messaging page)
SOURCES += \
    chat/chat_page.cpp

# Shared utilities
SOURCES += \
    shared/language_manager.cpp \
    shared/narration_manager.cpp

# Header files
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
    chat/chat_page.h \
    shared/language_manager.h \
    shared/narration_manager.h

# Include paths (relative to src/)
INCLUDEPATH += \
    . \
    home \
    capture \
    friends \
    player \
    profile \
    chat \
    shared

# Qt resource collections
RESOURCES += \
    capture/resources.qrc \
    home/home.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

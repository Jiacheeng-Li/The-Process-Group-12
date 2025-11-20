QT += core gui widgets multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MobileMainPage
TEMPLATE = app

# 源文件列表
SOURCES += \
    main.cpp \
    mainwindow.cpp \
    videowindow.cpp \
    the_player.cpp \
    the_button.cpp

# 头文件列表
HEADERS += \
    mainwindow.h \
    videowindow.h \
    the_player.h \
    the_button.h

# 资源文件列表
RESOURCES += \
    main.qrc

# 编译配置
CONFIG += c++11
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MobileMainPage  # 程序名称
TEMPLATE = app           # 应用程序模板

# 源文件列表（.cpp）
SOURCES += \
    main.cpp \
    mainwindow.cpp \
    videowindow.cpp

# 头文件列表（.h）
HEADERS += \
    mainwindow.h \
    videowindow.h

# 资源文件列表（.qrc）
RESOURCES += \
    main.qrc

# 移动端适配配置
android {
    # Android平台：强制竖屏
    QT += androidextras
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
    # 在android/AndroidManifest.xml中设置屏幕方向为portrait（后续部署时添加）
}

ios {
    # iOS平台：强制竖屏
    QMAKE_IOS_DEVICE_ARCHS = arm64
    QMAKE_IOS_SIMULATOR_ARCHS = x86_64
    # 在Info.plist中设置UISupportedInterfaceOrientations为竖屏
}

# 高DPI适配（解决不同分辨率屏幕模糊问题）
QT += core gui
CONFIG += highdpi_scaling

QT       += core gui widgets

TARGET = TikTokProfile
TEMPLATE = app

# 源文件（新增聊天相关cpp）
SOURCES += main.cpp \
    ChatModule.cpp \
           Profile.cpp \

# 头文件（新增聊天相关h）
HEADERS  += \
            ChatModule.h \
            Profile.h

# 资源文件（可选，用于存放聊天头像等）
# RESOURCES += resources.qrc

# 编译选项
QMAKE_CXXFLAGS += -std=c++11

# 目标路径
DESTDIR = ./bin
MOC_DIR = ./build/moc
OBJECTS_DIR = ./build/obj
RCC_DIR = ./build/rcc

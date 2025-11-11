#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // 强制应用竖屏显示（关键：移动端适配）
    a.setAttribute(Qt::AA_EnableHighDpiScaling); // 高DPI适配
    a.setApplicationName("MobilePage");

    MainWindow w;
    w.show();

    return a.exec();
}
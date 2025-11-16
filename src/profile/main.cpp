#include <QApplication>
#include "ChatModule.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // 全局基础样式
    a.setStyleSheet(R"(
        QWidget { font-family: sans-serif; background-color: #fff; }
        QScrollBar:vertical { width: 6px; background: #f5f5f5; }
        QScrollBar::handle:vertical { background: #ccc; border-radius: 3px; }
    )");

    // 启动页面切换管理器
    PageSwitcher w;
    w.setMinimumSize(600, 800);
    w.setWindowTitle("简化版界面");
    w.show();

    return a.exec();
}

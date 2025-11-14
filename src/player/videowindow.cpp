#include "videowindow.h"
#include <QLabel>
#include <QVBoxLayout>

VideoWindow::VideoWindow(QWidget *parent) : QWidget(parent)
{
    // 设置窗口标题和大小
    setWindowTitle("视频播放器");
    setMinimumSize(320, 240); // 简单设置一个初始大小

    // 空窗口提示（可选）
    QLabel *tipLabel = new QLabel("视频播放器窗口（空窗口）", this);
    tipLabel->setAlignment(Qt::AlignCenter);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(tipLabel);
}
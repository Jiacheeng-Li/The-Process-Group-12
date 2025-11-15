#include "recordpage.h"
#include <QResizeEvent>

RecordPage::RecordPage(QWidget *parent)
    : QWidget(parent)
{
    // ---------------- 手机外框 ----------------
    phoneFrame = new QWidget(this);
    phoneFrame->setStyleSheet(
        "background-color: #f0f0f0;"
        "border: 4px solid #222;"
        "border-radius: 20px;"
        );

    // ---------------- 后置摄像头区域（大矩形） ----------------
    backCamera = new QWidget(phoneFrame);
    backCamera->setStyleSheet("background-color: black;");

    // ---------------- 前置摄像头区域（小矩形） ----------------
    frontCamera = new QWidget(backCamera);   // 放在大矩形里面
    frontCamera->setStyleSheet(
        "background-color: #999;"
        "border: 2px solid white;"
        "border-radius: 8px;"
        );

    // ---------------- 红色录制按钮（放在大矩形内底部居中） ----------------
    recordButton = new QPushButton(backCamera);   // 父对象 = backCamera
    recordButton->setStyleSheet(
        "background-color: red;"
        "border-radius: 30px;"
        "border: 3px solid white;"
        );
    recordButton->setFixedSize(60, 60);
    connect(recordButton, &QPushButton::clicked,
            this, &RecordPage::onRecordButtonClicked);

    // ---------------- 右下角切换按钮（小圆） ----------------
    switchButton = new QPushButton(backCamera);
    switchButton->setStyleSheet(
        "background-color: rgba(255,255,255,220);"
        "border-radius: 18px;"
        "border: 2px solid #444;"
        );
    switchButton->setFixedSize(36, 36);
    connect(switchButton, &QPushButton::clicked,
            this, &RecordPage::onSwitchButtonClicked);

    // ---------------- 红点“呼吸灯”动画 ----------------
    connect(&pulseTimer, &QTimer::timeout, [this]() {
        pulseState = 1 - pulseState;
        if (pulseState)
            recordButton->setStyleSheet(
                "background-color: #d80000;"
                "border-radius: 30px;"
                "border: 3px solid white;"
                );
        else
            recordButton->setStyleSheet(
                "background-color: red;"
                "border-radius: 30px;"
                "border: 3px solid white;"
                );
    });
    pulseTimer.setInterval(500);
}

void RecordPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    int W = width();
    int H = height();

    // -------- 计算手机外框（9:16 竖屏） --------
    int frameH = static_cast<int>(H * 0.72);
    double aspect = 9.0 / 16.0;
    int frameW = static_cast<int>(frameH * aspect);

    if (frameW > W * 0.7) {
        frameW = static_cast<int>(W * 0.7);
        frameH = static_cast<int>(frameW / aspect);
    }

    int frameX = (W - frameW) / 2;
    int frameY = 20;
    phoneFrame->setGeometry(frameX, frameY, frameW, frameH);

    // -------- 大矩形（后置摄像头）填满手机内部（留一点边距） --------
    int margin = 10;
    int bw = frameW - 2 * margin;
    int bh = frameH - 2 * margin;
    backCamera->setGeometry(margin, margin, bw, bh);

    // -------- 小矩形（前置摄像头）放在大矩形左上角 --------
    int innerMargin = 16;
    int fw = bw / 3;                         // 宽度约 1/3
    int fh = static_cast<int>(fw * 16 / 9.0); // 也是竖长方形
    if (fh > bh / 2) fh = bh / 2;
    frontCamera->setGeometry(innerMargin, innerMargin, fw, fh);

    // -------- 红色录制按钮：大矩形内部底部居中 --------
    int rbSize = recordButton->width();
    int rbX = (bw - rbSize) / 2;
    int rbY = bh - rbSize - 16;              // 离底部 16 像素
    recordButton->move(rbX, rbY);

    // -------- 切换按钮：大矩形内部右下角 --------
    int sbSize = switchButton->width();
    int sbX = bw - sbSize - 16;
    int sbY = bh - sbSize - 16;
    switchButton->move(sbX, sbY);
}

void RecordPage::onRecordButtonClicked()
{
    if (!isRecording) {
        // 第一次点击：开始录制效果（仅 UI）
        isRecording = true;
        pulseTimer.start();
    } else {
        // 第二次点击：结束录制，之后可以在外部连接到发布页
        isRecording = false;
        pulseTimer.stop();
        // 还原红色按钮样式
        recordButton->setStyleSheet(
            "background-color: red;"
            "border-radius: 30px;"
            "border: 3px solid white;"
            );
        emit recordingFinished();
    }
}

void RecordPage::onSwitchButtonClicked()
{
    // 颜色互换：大矩形和小矩形黑 / 灰交换
    backIsBlack = !backIsBlack;

    if (backIsBlack) {
        backCamera->setStyleSheet("background-color: black;");
        frontCamera->setStyleSheet(
            "background-color: #999;"
            "border: 2px solid white;"
            "border-radius: 8px;"
            );
    } else {
        backCamera->setStyleSheet("background-color: #999;");
        frontCamera->setStyleSheet(
            "background-color: black;"
            "border: 2px solid white;"
            "border-radius: 8px;"
            );
    }
}

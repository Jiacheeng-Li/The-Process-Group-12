#include "recordpage.h"
#include <QResizeEvent>

RecordPage::RecordPage(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(1260, 760);

    phoneFrame = new QWidget(this);
    phoneFrame->setStyleSheet(
        "background-color: #f0f0f0;"
        "border: 4px solid #222;"
        "border-radius: 20px;"
        );

    backCamera = new QWidget(phoneFrame);
    backCamera->setStyleSheet("background-color: black;");

    frontCamera = new QWidget(backCamera);
    frontCamera->setStyleSheet(
        "background-color: #999;"
        "border: 2px solid white;"
        "border-radius: 8px;"
        );

    recordButton = new QPushButton(backCamera);
    recordButton->setStyleSheet(
        "background-color: red;"
        "border-radius: 30px;"
        "border: 3px solid white;"
        );
    recordButton->setFixedSize(60, 60);
    connect(recordButton, &QPushButton::clicked,
            this, &RecordPage::onRecordButtonClicked);

    switchButton = new QPushButton(backCamera);
    switchButton->setStyleSheet(
        "background-color: rgba(255,255,255,220);"
        "border-radius: 18px;"
        "border: 2px solid #444;"
        );
    switchButton->setFixedSize(36, 36);
    connect(switchButton, &QPushButton::clicked,
            this, &RecordPage::onSwitchButtonClicked);

    connect(&pulseTimer, &QTimer::timeout, [this]() {
        pulseState = 1 - pulseState;
        recordButton->setStyleSheet(
            QString("background-color:%1;border-radius:30px;border:3px solid white;")
                .arg(pulseState ? "#d80000" : "red")
            );
    });
    pulseTimer.setInterval(500);
}

void RecordPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    int W = width();
    int H = height();

    int frameH = static_cast<int>(H * 0.72);
    double aspect = 9.0 / 16.0;
    int frameW = static_cast<int>(frameH * aspect);

    if (frameW > W * 0.7) {
        frameW = static_cast<int>(W * 0.7);
        frameH = static_cast<int>(frameW / aspect);
    }

    int frameX = (W - frameW) / 2;
    int frameY = 22;
    phoneFrame->setGeometry(frameX, frameY, frameW, frameH);

    int margin = 10;
    backCamera->setGeometry(margin, margin, frameW - 2 * margin, frameH - 2 * margin);

    int fw = (frameW - 2 * margin) / 3;
    int fh = static_cast<int>(fw * 16 / 9.0);
    if (fh > (frameH - 2 * margin) / 2)
        fh = (frameH - 2 * margin) / 2;
    frontCamera->setGeometry(20, 20, fw, fh);

    int rbSize = recordButton->width();
    recordButton->move((frameW - 2 * margin - rbSize) / 2, (frameH - 2 * margin - rbSize - 18));

    int sbSize = switchButton->width();
    switchButton->move(frameW - 2 * margin - sbSize - 18, frameH - 2 * margin - sbSize - 18);
}

void RecordPage::onRecordButtonClicked()
{
    if (!isRecording)
    {
        isRecording = true;
        pulseTimer.start();
    }
    else
    {
        isRecording = false;
        pulseTimer.stop();
        recordButton->setStyleSheet(
            "background-color: red;"
            "border-radius: 30px;"
            "border: 3px solid white;"
            );
        emit recordingFinished();    // 🚀 进入发布页
    }
}

void RecordPage::onSwitchButtonClicked()
{
    backIsBlack = !backIsBlack;

    if (backIsBlack) {
        backCamera->setStyleSheet("background-color: black;");
        frontCamera->setStyleSheet(
            "background-color:#999; border:2px solid white; border-radius:8px;"
            );
    } else {
        backCamera->setStyleSheet("background-color:#999;");
        frontCamera->setStyleSheet(
            "background-color:black; border:2px solid white; border-radius:8px;"
            );
    }
}

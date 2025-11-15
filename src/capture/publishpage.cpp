#include "publishpage.h"
#include <QResizeEvent>
#include <QMessageBox>

PublishPage::PublishPage(QWidget *parent)
    : QWidget(parent)
{
    // ---------- 第一行 ----------
    inputBar = new QLineEdit(this);
    inputBar->setPlaceholderText("Write something...");
    inputBar->setStyleSheet(
        "background-color: white;"
        "border-radius: 6px;"
        "padding-left: 10px;"
        "font-size: 18px;"
        );

    bar5 = new QPushButton("5", this);
    bar6 = new QPushButton("✖", this);   // ★ 6 → ❌
    bar7 = new QPushButton("7", this);

    QList<QPushButton*> barButtons = {bar5, bar6, bar7};
    for (auto *b : barButtons) {
        b->setFixedHeight(38);
        b->setStyleSheet(
            "background-color: white;"
            "border-radius: 6px;"
            "font-size: 20px;"
            );
    }
    connect(bar6, &QPushButton::clicked, this, [this]() {
        emit backToRecord();        // 返回录制页面
    });


    // ---------- 第二行 ----------
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
        "border-radius: 8px;"
        "border: 2px solid white;"
        );


    // ---------- 第三行 ----------
    btn1 = new QPushButton("#", this);
    btn2 = new QPushButton("2", this);
    btn3 = new QPushButton("3", this);
    btn4 = new QPushButton("4", this);

    QList<QPushButton*> tags = {btn1, btn2, btn3, btn4};
    for (auto *b : tags) {
        b->setFixedHeight(38);
        b->setStyleSheet(
            "background-color: white;"
            "border-radius: 6px;"
            "font-size: 20px;"
            );
    }


    // ---------- 第四行 ----------
    sendButton = new QPushButton("Send >>>", this);
    sendButton->setFixedHeight(46);
    sendButton->setStyleSheet(
        "background-color: #d0d0d0;"
        "border-radius: 10px;"
        "font-size: 20px;"
        );

    connect(sendButton, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "Upload", "Successfully uploaded!");
        emit sendPressed();
    });
}


void PublishPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    int W = width();
    int H = height();

    // ---------- 第二行：手机框 ----------
    int frameH = int(H * 0.55);
    int frameW = int(frameH * 9.0 / 16.0);
    if (frameW > W * 0.75) {
        frameW = int(W * 0.75);
        frameH = int(frameW * 16.0 / 9.0);
    }
    int frameX = (W - frameW) / 2;
    int frameY = 120;  // 保持原来的间距
    phoneFrame->setGeometry(frameX, frameY, frameW, frameH);

    int margin = 10;
    int bw = frameW - margin * 2;
    int bh = frameH - margin * 2;
    backCamera->setGeometry(margin, margin, bw, bh);

    int fw = bw / 3;
    int fh = int(fw * 16 / 9.0);
    if (fh > bh / 2) fh = bh / 2;
    frontCamera->setGeometry(16, 16, fw, fh);


    // ---------- 第一行 ----------
    int barW = bw;
    int barX = frameX + (frameW - barW) / 2;
    int barY = frameY - 80;
    inputBar->setGeometry(barX, barY, barW, 40);

    int spacing = 10;
    int itemW = (barW - spacing * 2) / 3;
    int btnY = barY + 45;

    bar5->setGeometry(barX, btnY, itemW, 38);
    bar6->setGeometry(barX + itemW + spacing, btnY, itemW, 38);
    bar7->setGeometry(barX + (itemW + spacing) * 2, btnY, itemW, 38);


    // ---------- 第三行 ----------
    int tagY = frameY + frameH + 20;
    int tagW = (barW - spacing * 3) / 4;

    btn1->setGeometry(barX, tagY, tagW, 38);
    btn2->setGeometry(barX + tagW + spacing, tagY, tagW, 38);
    btn3->setGeometry(barX + (tagW + spacing) * 2, tagY, tagW, 38);
    btn4->setGeometry(barX + (tagW + spacing) * 3, tagY, tagW, 38);


    // ---------- 第四行 ----------
    int sendY = tagY + 50;
    sendButton->setGeometry(barX, sendY, barW, sendButton->height());
}

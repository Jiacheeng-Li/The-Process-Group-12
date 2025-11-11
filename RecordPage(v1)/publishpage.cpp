//zyx publishpage.cpp
#include "publishpage.h"

PublishPage::PublishPage(const QString &videoPath, QWidget *parent)
    : QWidget(parent) {

    player = new QMediaPlayer(this);
    QVideoWidget *preview = new QVideoWidget(this);
    player->setVideoOutput(preview);
    player->setMedia(QUrl::fromLocalFile(videoPath));
    player->play();

    QPushButton *publishBtn = new QPushButton("Publish");
    QPushButton *backBtn = new QPushButton("Back");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(preview);
    layout->addWidget(publishBtn);
    layout->addWidget(backBtn);
    setLayout(layout);

    //  新增：信号连接
    connect(publishBtn, &QPushButton::clicked, this, &PublishPage::publishVideo);
    connect(backBtn, &QPushButton::clicked, this, &PublishPage::backRequested);
}

void PublishPage::publishVideo() {
    QMessageBox::information(this, "Publish", "Video uploaded successfully!");
}

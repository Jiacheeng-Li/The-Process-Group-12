#include "video_viewer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>

VideoViewer::VideoViewer(const QString &coverPath, QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Video Viewer");
    setMinimumSize(780, 1080);
    setStyleSheet("background-color: black;");

    videoLabel = new QLabel(this);
    videoLabel->setScaledContents(true);
    videoLabel->setPixmap(QPixmap(coverPath).scaled(900, 1200, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));

    // --- Buttons ---
    closeBtn = new QPushButton(" ✖ ");
    likeBtn = new QPushButton(" ❤ 0");
    shareBtn = new QPushButton(" 🔁 0");

    closeBtn->setStyleSheet("font-size:18px; background:#222; color:white; padding:6px 16px; border-radius:6px;");
    likeBtn->setStyleSheet("font-size:16px; background:white; color:black; padding:6px 20px; border-radius:6px;");
    shareBtn->setStyleSheet("font-size:16px; background:white; color:black; padding:6px 20px; border-radius:6px;");

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addWidget(likeBtn);
    btnLayout->addWidget(shareBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);

    // --- Main layout ---
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(videoLabel, 1);
    mainLayout->addLayout(btnLayout);

    // slots
    connect(closeBtn, &QPushButton::clicked, this, &VideoViewer::onClosePressed);
    connect(likeBtn, &QPushButton::clicked, this, &VideoViewer::onLikePressed);
    connect(shareBtn, &QPushButton::clicked, this, &VideoViewer::onSharePressed);
}

void VideoViewer::onClosePressed()
{
    emit closed();
    close();
}

void VideoViewer::onLikePressed()
{
    likeCount++;
    likeBtn->setText(QString(" ❤ %1").arg(likeCount));
    emit liked();
}

void VideoViewer::onSharePressed()
{
    shareCount++;
    shareBtn->setText(QString(" 🔁 %1").arg(shareCount));
    emit shared();
}

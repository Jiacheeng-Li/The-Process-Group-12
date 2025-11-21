#include "frienditem.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPixmap>
#include <QMouseEvent>
#include <QPainter>

FriendItem::FriendItem(const QString &avatarPath,
                       const QString &username,
                       const QString &videoThumb,
                       const QDateTime &time,
                       QWidget *parent)
    : QWidget(parent), thumbPath(videoThumb)
{
    setStyleSheet("background:white; border-bottom: 1px solid #ddd; padding: 8px;");

    // 圆形头像
    avatar = new QLabel(this);
    avatar->setFixedSize(48, 48);
    avatar->setScaledContents(true);
    avatar->setPixmap(QPixmap(avatarPath).scaled(48, 48, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    avatar->setStyleSheet("border-radius:24px;");        // 让头像真正圆形

    avatar->installEventFilter(this);

    usernameLbl = new QLabel(username);
    usernameLbl->setStyleSheet("font-weight:bold; font-size:15px;");

    timeLbl = new QLabel(time.toString("hh:mm\nyyyy.MM.dd"));
    timeLbl->setAlignment(Qt::AlignRight);
    timeLbl->setStyleSheet("color:#555; font-size:12px;");

    QHBoxLayout *top = new QHBoxLayout;
    top->addWidget(avatar);
    top->addSpacing(6);
    top->addWidget(usernameLbl);
    top->addStretch();
    top->addWidget(timeLbl);

    thumbLbl = new QLabel;
    thumbLbl->setFixedSize(280, 420);
    thumbLbl->setScaledContents(true);
    thumbLbl->setPixmap(QPixmap(videoThumb));
    thumbLbl->setStyleSheet("border-radius:10px;");

    thumbLbl->installEventFilter(this);

    likeBtn = new QPushButton("♡ 0");
    commentBtn = new QPushButton("💬 0");
    shareBtn = new QPushButton("🔁 0");

    likeBtn->setCursor(Qt::PointingHandCursor);
    commentBtn->setCursor(Qt::PointingHandCursor);
    shareBtn->setCursor(Qt::PointingHandCursor);

    connect(likeBtn, &QPushButton::clicked, this, &FriendItem::onLike);
    connect(commentBtn, &QPushButton::clicked, this, &FriendItem::onComment);
    connect(shareBtn, &QPushButton::clicked, this, &FriendItem::onShare);

    QHBoxLayout *bottom = new QHBoxLayout;
    bottom->addWidget(likeBtn);
    bottom->addWidget(commentBtn);
    bottom->addWidget(shareBtn);
    bottom->addStretch();

    QVBoxLayout *main = new QVBoxLayout(this);
    main->setContentsMargins(8, 8, 8, 8);
    main->addLayout(top);
    main->addWidget(thumbLbl, 0, Qt::AlignLeft);
    main->addLayout(bottom);
}

FriendItem* FriendItem::fromPublish(const QString &videoThumb)
{
    return new FriendItem(":/icons/me.png", "Me", videoThumb, QDateTime::currentDateTime());
}

void FriendItem::onLike()
{
    liked = !liked;
    likeCount += liked ? 1 : -1;
    updateCountDisplay();
}

void FriendItem::onShare()
{
    shared = !shared;
    shareCount += shared ? 1 : -1;
    updateCountDisplay();
}

void FriendItem::onComment()
{
    commentCount++;
    updateCountDisplay();
    emit commentRequested(this);
}

void FriendItem::addComment(const QString &)
{
    commentCount++;
    updateCountDisplay();
}

void FriendItem::updateCountDisplay()
{
    likeBtn->setText(QString("%1 %2").arg(liked ? "❤️" : "♡").arg(likeCount));
    shareBtn->setText(QString("%1 %2").arg(shared ? "🔄" : "🔁").arg(shareCount));
    commentBtn->setText(QString("💬 %1").arg(commentCount));
}

bool FriendItem::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        if (watched == avatar)
        {
            emit avatarClicked(usernameLbl->text());
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

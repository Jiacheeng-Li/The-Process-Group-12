#include "friendspage.h"
#include <QInputDialog>
#include <QRandomGenerator>

FriendsPage::FriendsPage(QWidget *parent)
    : QWidget(parent)
{
    setStyleSheet("background:#F2F2F2;");

    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("border:none;");

    listWidget = new QWidget;
    listLayout = new QVBoxLayout(listWidget);
    listLayout->setAlignment(Qt::AlignTop);
    listLayout->setSpacing(12);
    listLayout->setContentsMargins(12, 12, 12, 12);

    scrollArea->setWidget(listWidget);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->setContentsMargins(0, 0, 0, 0);
    main->addWidget(scrollArea);

    // 自动生成 40 条朋友圈
    QStringList users = {"Alice", "Bob", "Ethan", "Luna", "Olivia", "James", "Lucas", "Sophia"};
    for (int i = 0; i < 40; ++i)
    {
        QString user = users.at(QRandomGenerator::global()->bounded(users.size()));
        QString video = QString(":/videos/%1.jpg").arg((i % 8) + 1);
        QDateTime t = QDateTime::currentDateTime().addSecs(-i * 600);

        FriendItem *item = new FriendItem(":/avatars/default.png", user, video, t);
        connect(item, &FriendItem::commentRequested, this, &FriendsPage::onCommentRequested);
        listLayout->addWidget(item);
    }
}

void FriendsPage::addNewPost(const QString &videoThumb)
{
    FriendItem *item = FriendItem::fromPublish(videoThumb);
    connect(item, &FriendItem::commentRequested, this, &FriendsPage::onCommentRequested);
    listLayout->insertWidget(0, item);
}

void FriendsPage::onCommentRequested(FriendItem *item)
{
    bool ok = false;
    QString text = QInputDialog::getText(this, "评论", "输入评论：",
                                         QLineEdit::Normal, "", &ok);
    if (!ok || text.trimmed().isEmpty()) return;
    item->addComment(text);
}

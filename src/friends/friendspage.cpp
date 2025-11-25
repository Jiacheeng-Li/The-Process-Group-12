#include "friendspage.h"
#include <QInputDialog>
#include <QRandomGenerator>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QImageReader>
#include <QPixmap>
#include <QApplication>
#include <QFile>
#include "../player/the_button.h"

FriendsPage::FriendsPage(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("friendsPage");
    setStyleSheet(
        "QWidget#friendsPage { background-color: #00040d; }"
        "QScrollArea { background: transparent; border: none; }"
        "QScrollArea > QWidget > QWidget { background: transparent; }"
    );

    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("border:none; background: transparent;");
    scrollArea->viewport()->setStyleSheet("background: transparent;");

    listWidget = new QWidget;
    listLayout = new QVBoxLayout(listWidget);
    listLayout->setAlignment(Qt::AlignTop);
    listLayout->setSpacing(12);
    listLayout->setContentsMargins(12, 12, 12, 12);

    scrollArea->setWidget(listWidget);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->setContentsMargins(0, 0, 0, 0);
    main->addWidget(scrollArea);
}

FriendsPage::FriendsPage(const std::vector<TheButtonInfo> &videos, QWidget *parent)
    : QWidget(parent)
{
    setObjectName("friendsPage");
    setStyleSheet(
        "QWidget#friendsPage { background-color: #00040d; }"
        "QScrollArea { background: transparent; border: none; }"
        "QScrollArea > QWidget > QWidget { background: transparent; }"
    );

    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("border:none; background: transparent;");
    scrollArea->viewport()->setStyleSheet("background: transparent;");

    listWidget = new QWidget;
    listLayout = new QVBoxLayout(listWidget);
    listLayout->setAlignment(Qt::AlignTop);
    listLayout->setSpacing(12);
    listLayout->setContentsMargins(12, 12, 12, 12);

    scrollArea->setWidget(listWidget);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->setContentsMargins(0, 0, 0, 0);
    main->addWidget(scrollArea);

    initializeWithVideos(videos);
}

void FriendsPage::setVideos(const std::vector<TheButtonInfo> &videos)
{
    // 清空现有内容
    QLayoutItem* item;
    while ((item = listLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    
    initializeWithVideos(videos);
}

void FriendsPage::initializeWithVideos(const std::vector<TheButtonInfo> &videos)
{
    videoList = videos;
    
    if (videos.empty()) {
        // 如果没有视频，生成一些模拟内容
        QStringList users = {"Alice", "Bob", "Ethan", "Luna", "Olivia", "James", "Lucas", "Sophia"};
        for (int i = 0; i < 10; ++i) {
            QString user = users.at(QRandomGenerator::global()->bounded(users.size()));
            QDateTime t = QDateTime::currentDateTime().addSecs(-i * 600);
            // 创建一个空的缩略图路径
            FriendItem *item = new FriendItem("", user, "", t);
            connect(item, &FriendItem::commentRequested, this, &FriendsPage::onCommentRequested);
            listLayout->addWidget(item);
        }
        return;
    }

    // 使用实际的视频缩略图创建朋友圈动态
    QStringList users = {"Alice", "Bob", "Ethan", "Luna", "Olivia", "James", "Lucas", "Sophia", "Emma", "Noah", "Mia", "Oliver"};
    QStringList avatarColors = {"#2f8dff", "#ff6b6b", "#4ecdc4", "#ffe66d", "#ff9ff3", "#54a0ff", "#5f27cd", "#00d2d3"};

    // 生成 30-40 条朋友圈动态
    int numPosts = qMin(40, static_cast<int>(videos.size() * 5));  // 如果视频少，生成更多动态
    for (int i = 0; i < numPosts; ++i) {
        QString user = users.at(QRandomGenerator::global()->bounded(users.size()));
        
        // 从视频列表中选择一个缩略图（循环使用）
        const TheButtonInfo &videoInfo = videos.at(i % videos.size());
        QString thumbPath;
        QPixmap thumbPix;
        bool hasThumbnail = false;
        
        // 首先尝试从视频URL获取缩略图PNG文件路径
        if (videoInfo.url && videoInfo.url->isLocalFile()) {
            QString videoPath = videoInfo.url->toLocalFile();
            QString pngPath = videoPath.left(videoPath.length() - 4) + ".png";
            if (QFile::exists(pngPath)) {
                thumbPath = pngPath;
                thumbPix = QPixmap(pngPath);
                if (!thumbPix.isNull()) {
                    hasThumbnail = true;
                }
            }
        }
        
        // 如果没有找到PNG文件，尝试使用图标
        if (!hasThumbnail && videoInfo.icon) {
            thumbPix = videoInfo.icon->pixmap(280, 420);
            if (!thumbPix.isNull()) {
                hasThumbnail = true;
            }
        }
        
        // 生成随机时间
        QDateTime t = QDateTime::currentDateTime().addSecs(-i * (QRandomGenerator::global()->bounded(300) + 300));
        
        // 创建头像路径（空字符串，FriendItem会自动创建彩色占位符）
        QString avatarPath = "";
        
        FriendItem *item = new FriendItem(avatarPath, user, thumbPath, t);
        
        // 如果有缩略图，设置它
        if (hasThumbnail) {
            item->setThumbnail(thumbPix);
        }
        
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

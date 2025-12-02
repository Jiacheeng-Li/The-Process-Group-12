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
#include <QResizeEvent>
#include <QTimer>
#include "../player/the_button.h"
#include "../shared/language_manager.h"
#include "../shared/narration_manager.h"
#include <QVector>
#include <QApplication>

namespace {
struct TagEntry {
    QString zh;
    QString en;
};

struct CaptionEntry {
    QString zhTemplate;
    QString enTemplate;
};

const QVector<TagEntry> kTagPool = {
    {"#旅行日记", "#TravelDiary"},
    {"#美食闪现", "#FoodCravings"},
    {"#治愈瞬间", "#HealingMoment"},
    {"#运动打卡", "#WorkoutLog"},
    {"#城市夜景", "#CityLights"},
    {"#电影情绪", "#CinemaMood"},
    {"#学习日常", "#StudyFlow"},
    {"#音乐小宇宙", "#TinyMusicWorld"},
    {"#复古滤镜", "#RetroFrame"},
    {"#随手拍", "#SnapAndGo"}
};

const QVector<CaptionEntry> kCaptionPool = {
    {"%1 刚刚记录了生活里的微光，愿你也被点亮。", "%1 just captured a spark of real life and hopes it finds you."},
    {"新的灵感来自一杯咖啡和一段旅程。", "Fresh inspo from a coffee break and a tiny trip."},
    {"被这段画面治愈到，分享给你。", "Needed to share this healing little scene."},
    {"%1 的今日份小确幸上线啦。", "%1 is dropping today’s small joy."},
    {"一路向前的风景，值得收藏。", "Roadside views worth saving."},
    {"想把这段旋律留在这里，等你来听。", "Parking this melody here for you."},
    {"有些瞬间不分享会发光，所以发出来啦。", "Some moments glow brighter when shared."},
    {"%1 继续更新日常，希望你喜欢。", "%1 keeps the daily log rolling, hope you vibe with it."}
};

FriendPostCopy randomCopyForUser(const QString &user)
{
    FriendPostCopy copy;
    if (!kTagPool.isEmpty()) {
        const auto &tag = kTagPool.at(QRandomGenerator::global()->bounded(kTagPool.size()));
        copy.tagZh = tag.zh;
        copy.tagEn = tag.en;
    }
    if (!kCaptionPool.isEmpty()) {
        const auto &caption = kCaptionPool.at(QRandomGenerator::global()->bounded(kCaptionPool.size()));
        QString zh = caption.zhTemplate;
        QString en = caption.enTemplate;
        if (zh.contains("%1")) {
            zh = zh.arg(user);
        }
        if (en.contains("%1")) {
            en = en.arg(user);
        }
        copy.captionZh = zh;
        copy.captionEn = en;
    }
    return copy;
}
} // namespace

FriendsPage::FriendsPage(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("friendsPage");
    setAttribute(Qt::WA_StyledBackground, true);

    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // Remove scrollbar
    scrollArea->setStyleSheet("border:none; background: transparent;");
    scrollArea->viewport()->setStyleSheet("background: transparent;");

    listWidget = new QWidget;
    listLayout = new QVBoxLayout(listWidget);
    listLayout->setAlignment(Qt::AlignTop);
    listLayout->setSpacing(12);
    listLayout->setContentsMargins(12, 12, 12, 12);

    scrollArea->setWidget(listWidget);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->setContentsMargins(24, 0, 12, 0);
    main->setSpacing(0);
    main->addWidget(scrollArea, 1); // Use stretch factor to let scrollArea fill space
    
    applyThemeStyles();
    updateResponsiveLayout();
}

FriendsPage::FriendsPage(const std::vector<TheButtonInfo> &videos, QWidget *parent)
    : QWidget(parent)
{
    setObjectName("friendsPage");
    setAttribute(Qt::WA_StyledBackground, true);

    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // Remove scrollbar
    scrollArea->setStyleSheet("border:none; background: transparent;");
    scrollArea->viewport()->setStyleSheet("background: transparent;");

    listWidget = new QWidget;
    listLayout = new QVBoxLayout(listWidget);
    listLayout->setAlignment(Qt::AlignTop);
    listLayout->setSpacing(12);
    listLayout->setContentsMargins(12, 12, 12, 12);

    scrollArea->setWidget(listWidget);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->setContentsMargins(24, 0, 12, 0);
    main->setSpacing(0);
    main->addWidget(scrollArea, 1); // Use stretch factor to let scrollArea fill space

    initializeWithVideos(videos);
    applyThemeStyles();
    updateResponsiveLayout();
}

void FriendsPage::setVideos(const std::vector<TheButtonInfo> &videos)
{
    // Clear existing content
    QLayoutItem* item;
    while ((item = listLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    
    initializeWithVideos(videos);
    propagateThemeToItems();
}

void FriendsPage::initializeWithVideos(const std::vector<TheButtonInfo> &videos)
{
    videoList = videos;
    
    if (videos.empty()) {
        // If no videos, generate some mock content
        QStringList users = {"Alice", "Bob", "Ethan", "Luna", "Olivia", "James", "Lucas", "Sophia"};
        for (int i = 0; i < 10; ++i) {
            QString user = users.at(QRandomGenerator::global()->bounded(users.size()));
            QDateTime t = QDateTime::currentDateTime().addSecs(-i * 600);
            FriendPostCopy copy = randomCopyForUser(user);
            // Create an empty thumbnail path
            FriendItem *item = new FriendItem("", user, "", t, -1, copy);
            item->setDayMode(dayMode_ && !highContrastMode_);
            connect(item, &FriendItem::commentRequested, this, &FriendsPage::onCommentRequested);
            listLayout->addWidget(item);
        }
        return;
    }

    // Use actual video thumbnails to create friend feed posts
    QStringList users = {"Alice", "Bob", "Ethan", "Luna", "Olivia", "James", "Lucas", "Sophia", "Emma", "Noah", "Mia", "Oliver"};
    QStringList avatarColors = {"#2f8dff", "#ff6b6b", "#4ecdc4", "#ffe66d", "#ff9ff3", "#54a0ff", "#5f27cd", "#00d2d3"};

    // Generate 30-40 friend feed posts
    int numPosts = qMin(40, static_cast<int>(videos.size() * 5));  // If fewer videos, generate more posts
    for (int i = 0; i < numPosts; ++i) {
        QString user = users.at(QRandomGenerator::global()->bounded(users.size()));
        FriendPostCopy copy = randomCopyForUser(user);
        
        // Select a thumbnail from video list (reuse cyclically)
        const TheButtonInfo &videoInfo = videos.at(i % videos.size());
        QString thumbPath;
        QPixmap thumbPix;
        bool hasThumbnail = false;
        
        // First try to get thumbnail PNG file path from video URL
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
        
        // If PNG file not found, try using icon
        if (!hasThumbnail && videoInfo.icon) {
            thumbPix = videoInfo.icon->pixmap(280, 420);
            if (!thumbPix.isNull()) {
                hasThumbnail = true;
            }
        }
        
        // Generate random time
        QDateTime t = QDateTime::currentDateTime().addSecs(-i * (QRandomGenerator::global()->bounded(300) + 300));
        
        // Avatar path left empty so FriendItem can generate a colored placeholder
        QString avatarPath = "";
        
        // Calculate video index (reuse cyclically)
        int videoIndex = i % videos.size();
        
        FriendItem *item = new FriendItem(avatarPath, user, thumbPath, t, videoIndex, copy);
        item->setDayMode(dayMode_ && !highContrastMode_);
        
        // If thumbnail exists, set it
        if (hasThumbnail) {
            item->setThumbnail(thumbPix);
        }
        
        connect(item, &FriendItem::commentRequested, this, &FriendsPage::onCommentRequested);
        connect(item, &FriendItem::avatarClicked, this, &FriendsPage::goToProfile);
        connect(item, &FriendItem::thumbnailClicked, this, [this](int index) {
            emit playVideoRequested(index);
        });
        
        // Randomly generate some comments (30% probability)
        if (QRandomGenerator::global()->bounded(100) < 30) {
            QStringList commenters = {"Alice", "Bob", "Ethan", "Luna", "Olivia", "James", "Lucas", "Sophia", "Emma", "Noah"};
            QStringList comments = {
                "太棒了！", "Awesome!", "喜欢这个！", "Love it!", "真不错", "Nice!", 
                "👍", "🔥", "太美了", "Beautiful!", "赞", "Great!"
            };
            int numComments = QRandomGenerator::global()->bounded(3) + 1; // 1-3 comments
            for (int j = 0; j < numComments; ++j) {
                QString commenter = commenters.at(QRandomGenerator::global()->bounded(commenters.size()));
                QString comment = comments.at(QRandomGenerator::global()->bounded(comments.size()));
                item->addComment(comment, commenter);
            }
        }
        
        listLayout->addWidget(item);
    }
}

void FriendsPage::addNewPost(const QString &videoThumb)
{
    FriendItem *item = FriendItem::fromPublish(videoThumb, randomCopyForUser("Me"));
    item->setDayMode(dayMode_ && !highContrastMode_);
    connect(item, &FriendItem::commentRequested, this, &FriendsPage::onCommentRequested);
    listLayout->insertWidget(0, item);
    
    // Ensure newly added item correctly sets thumbnail size after layout is complete
    // Use multiple delayed triggers to ensure widget is added to layout and has correct width
    QTimer::singleShot(50, item, [item, this]() {
        if (item && listWidget) {
            // Force update layout
            listWidget->updateGeometry();
            QApplication::processEvents();
        }
    });
    QTimer::singleShot(150, item, [item]() {
        if (item && item->width() > 0) {
            item->updateThumbnailSize();
        }
    });
    QTimer::singleShot(300, item, [item]() {
        if (item && item->width() > 0) {
            item->updateThumbnailSize();
        }
    });
}

void FriendsPage::onCommentRequested(FriendItem *item)
{
    auto &langMgr = LanguageManager::instance();
    const auto pick = [&](const QString &zh, const QString &en) {
        return langMgr.pick(zh, en);
    };

    bool ok = false;
    QString text = QInputDialog::getText(this,
                                         pick(QString::fromUtf8("评论"), QStringLiteral("Comment")),
                                         pick(QString::fromUtf8("输入评论："), QStringLiteral("Type your comment:")),
                                         QLineEdit::Normal, "", &ok);
    if (!ok || text.trimmed().isEmpty()) return;
    item->addComment(text);
    
    // Voice narration
    NarrationManager::instance().narrate(
        QString::fromUtf8("评论已添加"),
        "Comment added"
    );
}

void FriendsPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // Use debounce mechanism: delay update to avoid frequent layout adjustments
    static QTimer *debounceTimer = nullptr;
    if (!debounceTimer) {
        debounceTimer = new QTimer(this);
        debounceTimer->setSingleShot(true);
        debounceTimer->setInterval(100);  // 100ms debounce
        connect(debounceTimer, &QTimer::timeout, this, &FriendsPage::updateResponsiveLayout);
    }
    debounceTimer->stop();
    debounceTimer->start();
}

void FriendsPage::updateResponsiveLayout()
{
    if (!listLayout || !scrollArea) {
        return;
    }
    
    const int pageWidth = width();
    
    // Cache last width to avoid unnecessary updates
    static int lastPageWidth = -1;
    if (lastPageWidth == pageWidth) {
        return;  // Width hasn't changed, no need to update
    }
    lastPageWidth = pageWidth;
    
    // Responsive design: adjust layout based on page width
    if (pageWidth < 600) {
        // Small screen: smaller margins and spacing
        listLayout->setContentsMargins(8, 8, 8, 8);
        listLayout->setSpacing(8);
        
        // Adjust main layout margins
        if (layout()) {
            layout()->setContentsMargins(12, 0, 8, 0);
        }
    } else if (pageWidth < 1000) {
        // Medium screen: standard margins
        listLayout->setContentsMargins(12, 12, 12, 12);
        listLayout->setSpacing(12);
        
        if (layout()) {
            layout()->setContentsMargins(20, 0, 12, 0);
        }
    } else {
        // Large screen: fixed margins, no longer dynamically adjusted
        listLayout->setContentsMargins(16, 16, 16, 16);
        listLayout->setSpacing(16);
        
        if (layout()) {
            layout()->setContentsMargins(24, 0, 12, 0);
        }
    }
    
    // Notify all FriendItems to update thumbnail size to adapt to new window width
    // Use QTimer to delay execution, ensure update after layout adjustment is complete
    QTimer::singleShot(50, this, [this]() {
        if (!listWidget) {
            return;
        }
        // Find all FriendItems and update their thumbnail sizes
        QList<FriendItem*> items = listWidget->findChildren<FriendItem*>();
        for (FriendItem *item : items) {
            if (item) {
                item->updateThumbnailSize();
            }
        }
    });
}

void FriendsPage::setDayMode(bool dayMode)
{
    if (highContrastMode_) {
        dayMode_ = false;
    } else {
        dayMode_ = dayMode;
    }
    applyThemeStyles();
    propagateThemeToItems();
}

void FriendsPage::setHighContrastMode(bool enabled)
{
    if (highContrastMode_ == enabled) {
        return;
    }
    highContrastMode_ = enabled;
    if (highContrastMode_) {
        dayMode_ = false;
    }
    applyThemeStyles();
    propagateThemeToItems();
}

void FriendsPage::applyThemeStyles()
{
    QString style;
    if (highContrastMode_) {
        style = QStringLiteral(
            "QWidget#friendsPage {"
            "  background-color: #000000;"
            "  border: 3px solid #f4c430;"
            "  border-radius: 12px;"
            "}"
            "QScrollArea { background: transparent; border: none; }"
            "QScrollArea > QWidget > QWidget { background: transparent; }"
        );
    } else if (dayMode_) {
        style = QStringLiteral(
            "QWidget#friendsPage {"
            "  background-color: rgba(255,255,255,0.95);"
            "  border: 2px solid rgba(58,82,132,0.25);"
            "  border-radius: 12px;"
            "  box-shadow: 0 12px 30px rgba(15,35,80,0.08);"
            "}"
            "QScrollArea { background: transparent; border: none; }"
            "QScrollArea > QWidget > QWidget { background: transparent; }"
        );
    } else {
        style = QStringLiteral(
            "QWidget#friendsPage {"
            "  background-color: #00040d;"
            "  border: 3px solid #6CADFF;"
            "  border-radius: 12px;"
            "}"
            "QScrollArea { background: transparent; border: none; }"
            "QScrollArea > QWidget > QWidget { background: transparent; }"
        );
    }
    setStyleSheet(style);
}

void FriendsPage::propagateThemeToItems()
{
    if (!listWidget) {
        return;
    }
    const bool useDay = dayMode_ && !highContrastMode_;
    const auto items = listWidget->findChildren<FriendItem*>();
    for (FriendItem *item : items) {
        if (item) {
            item->setHighContrastMode(highContrastMode_);
            item->setDayMode(useDay);
        }
    }
}

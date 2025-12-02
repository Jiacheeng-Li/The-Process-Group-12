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
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 删除滚动条
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
    main->addWidget(scrollArea, 1); // 使用stretch factor让scrollArea填充空间
    
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
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 删除滚动条
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
    main->addWidget(scrollArea, 1); // 使用stretch factor让scrollArea填充空间

    initializeWithVideos(videos);
    applyThemeStyles();
    updateResponsiveLayout();
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
    propagateThemeToItems();
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
            FriendPostCopy copy = randomCopyForUser(user);
            // 创建一个空的缩略图路径
            FriendItem *item = new FriendItem("", user, "", t, -1, copy);
            item->setDayMode(dayMode_ && !highContrastMode_);
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
        FriendPostCopy copy = randomCopyForUser(user);
        
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
        
        // Avatar path left empty so FriendItem can generate a colored placeholder
        QString avatarPath = "";
        
        // 计算视频索引（循环使用）
        int videoIndex = i % videos.size();
        
        FriendItem *item = new FriendItem(avatarPath, user, thumbPath, t, videoIndex, copy);
        item->setDayMode(dayMode_ && !highContrastMode_);
        
        // 如果有缩略图，设置它
        if (hasThumbnail) {
            item->setThumbnail(thumbPix);
        }
        
        connect(item, &FriendItem::commentRequested, this, &FriendsPage::onCommentRequested);
        connect(item, &FriendItem::avatarClicked, this, &FriendsPage::goToProfile);
        connect(item, &FriendItem::thumbnailClicked, this, [this](int index) {
            emit playVideoRequested(index);
        });
        
        // 随机生成一些评论（30%的概率）
        if (QRandomGenerator::global()->bounded(100) < 30) {
            QStringList commenters = {"Alice", "Bob", "Ethan", "Luna", "Olivia", "James", "Lucas", "Sophia", "Emma", "Noah"};
            QStringList comments = {
                "太棒了！", "Awesome!", "喜欢这个！", "Love it!", "真不错", "Nice!", 
                "👍", "🔥", "太美了", "Beautiful!", "赞", "Great!"
            };
            int numComments = QRandomGenerator::global()->bounded(3) + 1; // 1-3条评论
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
    
    // 确保新添加的item在布局完成后正确设置缩略图尺寸
    // 使用多个延迟触发，确保widget已经添加到布局并获得了正确的宽度
    QTimer::singleShot(50, item, [item, this]() {
        if (item && listWidget) {
            // 强制更新布局
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
    
    // 语音播报
    NarrationManager::instance().narrate(
        QString::fromUtf8("评论已添加"),
        "Comment added"
    );
}

void FriendsPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // 使用防抖机制：延迟更新，避免频繁调整布局
    static QTimer *debounceTimer = nullptr;
    if (!debounceTimer) {
        debounceTimer = new QTimer(this);
        debounceTimer->setSingleShot(true);
        debounceTimer->setInterval(100);  // 100ms防抖
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
    
    // 缓存上次的宽度，避免不必要的更新
    static int lastPageWidth = -1;
    if (lastPageWidth == pageWidth) {
        return;  // 宽度没有变化，不需要更新
    }
    lastPageWidth = pageWidth;
    
    // 响应式设计：根据页面宽度调整布局
    if (pageWidth < 600) {
        // 小屏幕：较小的边距和间距
        listLayout->setContentsMargins(8, 8, 8, 8);
        listLayout->setSpacing(8);
        
        // 调整主布局边距
        if (layout()) {
            layout()->setContentsMargins(12, 0, 8, 0);
        }
    } else if (pageWidth < 1000) {
        // 中等屏幕：标准边距
        listLayout->setContentsMargins(12, 12, 12, 12);
        listLayout->setSpacing(12);
        
        if (layout()) {
            layout()->setContentsMargins(20, 0, 12, 0);
        }
    } else {
        // 大屏幕：固定边距，不再动态调整
        listLayout->setContentsMargins(16, 16, 16, 16);
        listLayout->setSpacing(16);
        
        if (layout()) {
            layout()->setContentsMargins(24, 0, 12, 0);
        }
    }
    
    // 通知所有 FriendItem 更新缩略图大小，以适应新的窗口宽度
    // 使用 QTimer 延迟执行，确保布局调整完成后再更新
    QTimer::singleShot(50, this, [this]() {
        if (!listWidget) {
            return;
        }
        // 查找所有 FriendItem 并更新它们的缩略图大小
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

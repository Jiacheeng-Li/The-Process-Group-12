#include "frienditem.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPixmap>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QFileInfo>
#include <QFont>
#include <QColor>
#include <QDir>
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QLineEdit>
#include <QPushButton>
#include <QFrame>
#include <QLabel>
#include <QResizeEvent>
#include <QShowEvent>
#include <QTimer>

// Resolve avatar path based on username
static QString getAvatarPathForUser(const QString &username)
{
    // Map the first five demo users to avatar files 1-5.jpg
    QStringList avatarUsers = {"Alice", "Bob", "Ethan", "Luna", "Olivia"};
    int index = avatarUsers.indexOf(username);
    
    if (index >= 0 && index < 5) {
        // 获取头像文件路径（相对于可执行文件或当前目录）
        QString appDir = QApplication::applicationDirPath();
        QString fileName = QString::number(index + 1) + ".jpg";
        
        // 尝试从多个可能的路径查找头像文件
        QStringList searchPaths = {
            QDir::currentPath() + "/friends/avatar/" + fileName,
            QDir::currentPath() + "/../friends/avatar/" + fileName,
            QDir::currentPath() + "/src/friends/avatar/" + fileName,  // 源码目录
            QDir::currentPath() + "/../src/friends/avatar/" + fileName,
            QDir::currentPath() + "/../../src/friends/avatar/" + fileName,
            appDir + "/friends/avatar/" + fileName,
            appDir + "/../friends/avatar/" + fileName,
            appDir + "/../../friends/avatar/" + fileName,
            appDir + "/../../src/friends/avatar/" + fileName,  // 构建目录上一层
            appDir + "/../../../src/friends/avatar/" + fileName, // 回到项目根
            "friends/avatar/" + fileName,
            "../friends/avatar/" + fileName,
            "src/friends/avatar/" + fileName  // 相对路径
        };
        
        for (const QString &path : searchPaths) {
            QString normalizedPath = QDir::cleanPath(path);
            if (QFile::exists(normalizedPath)) {
                qDebug() << "找到头像文件:" << normalizedPath << "for user" << username;
                return QDir::cleanPath(normalizedPath);
            }
        }
        
        qDebug() << "未找到头像文件 for user" << username << "fileName:" << fileName;
        qDebug() << "搜索路径:" << searchPaths;
    }
    
    return "";  // 返回空字符串表示使用纯色头像
}

// 完全照抄Profile页的roundedFromIcon逻辑，但适配图片路径
namespace {
QPixmap roundedFromPath(const QString &imagePath, const QSize &size, int radius) {
    QPixmap base(size);
    base.fill(Qt::transparent);

    QPixmap source;
    if (!imagePath.isEmpty() && QFile::exists(imagePath)) {
        source = QPixmap(imagePath);
    }

    if (!source.isNull()) {
        QPixmap pix = source.scaled(size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QPainter painter(&base);
        painter.setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        path.addRoundedRect(QRectF(0, 0, size.width(), size.height()), radius, radius);
        painter.setClipPath(path);
        painter.drawPixmap(0, 0, pix);
        painter.end();
    } else {
        base.fill(QColor("#2d2d2d"));
    }

    return base;
}
} // namespace

FriendItem::FriendItem(const QString &avatarPath,
                       const QString &username,
                       const QString &videoThumb,
                       const QDateTime &time,
                       int videoIndex,
                       const FriendPostCopy &copy,
                       QWidget *parent)
    : QWidget(parent),
      thumbPath(videoThumb),
      videoIndex_(videoIndex),
      copy_(copy),
      username_(username)
{
    setStyleSheet(
        "QWidget { "
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(2,8,20,0.92), stop:1 rgba(13,13,13,0.95)); "
        "  border: 2px solid #6CADFF; "
        "  border-radius: 12px; "
        "  padding: 0px; "
        "  margin: 8px 0px; "
        "}"
        "QLabel { color: white; }"
    );

    QVBoxLayout *main = new QVBoxLayout(this);
    main->setContentsMargins(0, 0, 0, 0);
    main->setSpacing(0);

    // 顶部：头像 + 用户名 + 时间
    QHBoxLayout *top = new QHBoxLayout;
    top->setContentsMargins(12, 12, 12, 8);
    top->setSpacing(10);

    // 圆形头像（完全照抄Profile页逻辑）
    avatar = new QLabel(this);
    avatar->setFixedSize(40, 40);
    avatar->setScaledContents(false);
    
    QString finalAvatarPath = avatarPath;
    if (finalAvatarPath.isEmpty() || !QFile::exists(finalAvatarPath)) {
        finalAvatarPath = getAvatarPathForUser(username);
    }
    
    // 使用Profile页相同的逻辑
    QPixmap avatarPix = roundedFromPath(finalAvatarPath, QSize(40, 40), 20);
    avatar->setPixmap(avatarPix);
    avatar->setStyleSheet(
        "border: 2px solid #6CADFF;"
        "border-radius: 20px;"
        "background-color: transparent;"
    );
    avatar->installEventFilter(this);

    usernameLbl = new QLabel(username);
    usernameLbl->setStyleSheet("font-weight:bold; font-size:16px; color: white;");

    timeLbl = new QLabel(time.toString("HH:mm"));
    timeLbl->setStyleSheet("color:#8aa7d9; font-size:13px;");

    top->addWidget(avatar);
    top->addWidget(usernameLbl);
    top->addStretch();
    top->addWidget(timeLbl);
    main->addLayout(top);

    // 视频缩略图（Instagram风格，响应式，保持16:9比例）
    thumbLbl = new QLabel;
    thumbLbl->setScaledContents(false); // 不使用自动缩放，手动控制比例
    thumbLbl->setAlignment(Qt::AlignCenter);
    thumbLbl->setStyleSheet("background: #0D0D0D;");
    thumbLbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    
    // 不设置固定高度，让它在第一次显示时根据实际宽度计算
    // 初始大小会在showEvent和resizeEvent中设置
    if (!videoThumb.isEmpty() && QFile::exists(videoThumb)) {
        QPixmap pixmap(videoThumb);
        if (!pixmap.isNull()) {
            // 缓存原始pixmap
            originalThumbPixmap_ = pixmap;
        } else {
            thumbLbl->setText(QString::fromUtf8("📹"));
            thumbLbl->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(108,173,255,0.2), stop:1 rgba(58,125,255,0.3)); color: white; font-size: 48px;");
        }
    } else {
        thumbLbl->setText(QString::fromUtf8("📹"));
        thumbLbl->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(108,173,255,0.2), stop:1 rgba(58,125,255,0.3)); color: white; font-size: 48px;");
    }
    thumbLbl->installEventFilter(this);
    main->addWidget(thumbLbl);

    // 操作按钮区域（Instagram风格：三个按钮平均分配，左右对齐视频）
    QHBoxLayout *actionLayout = new QHBoxLayout;
    actionLayout->setContentsMargins(12, 8, 12, 8);
    actionLayout->setSpacing(0);

    // Instagram风格的按钮样式
    QString buttonStyle = 
        "QPushButton {"
        "  background: transparent;"
        "  color: white;"
        "  border: none;"
        "  padding: 8px 12px;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "  text-align: left;"
        "}"
        "QPushButton:hover {"
        "  background: rgba(108,173,255,0.1);"
        "  border-radius: 4px;"
        "}";

    likeBtn = new QPushButton(this);
    likeBtn->setIcon(QIcon(":/icons/icons/like.svg"));
    likeBtn->setIconSize(QSize(22, 22));
    likeBtn->setText(" 0");
    likeBtn->setStyleSheet(buttonStyle);
    likeBtn->setCursor(Qt::PointingHandCursor);

    commentBtn = new QPushButton(this);
    commentBtn->setIcon(QIcon(":/icons/icons/comment.svg"));
    commentBtn->setIconSize(QSize(22, 22));
    commentBtn->setText(" 0");
    commentBtn->setStyleSheet(buttonStyle);
    commentBtn->setCursor(Qt::PointingHandCursor);

    shareBtn = new QPushButton(this);
    shareBtn->setIcon(QIcon(":/icons/icons/share.svg"));
    shareBtn->setIconSize(QSize(22, 22));
    shareBtn->setText(" 0");
    shareBtn->setStyleSheet(buttonStyle);
    shareBtn->setCursor(Qt::PointingHandCursor);

    repostBtn = new QPushButton(this);
    repostBtn->setIcon(QIcon(":/icons/icons/repost.svg"));
    repostBtn->setIconSize(QSize(22, 22));
    repostBtn->setText(" 0");
    repostBtn->setStyleSheet(buttonStyle);
    repostBtn->setCursor(Qt::PointingHandCursor);

    // 四个按钮平均分配宽度
    actionLayout->addWidget(likeBtn, 1);
    actionLayout->addWidget(commentBtn, 1);
    actionLayout->addWidget(shareBtn, 1);
    actionLayout->addWidget(repostBtn, 1);

    connect(likeBtn, &QPushButton::clicked, this, &FriendItem::onLike);
    connect(commentBtn, &QPushButton::clicked, this, &FriendItem::onComment);
    connect(shareBtn, &QPushButton::clicked, this, &FriendItem::onShare);
    connect(repostBtn, &QPushButton::clicked, this, &FriendItem::onRepost);

    main->addLayout(actionLayout);

    // 标签和内容
    tagLbl = new QLabel;
    tagLbl->setStyleSheet("color:#6CADFF; font-size:14px; font-weight:600; padding: 0px 12px;");
    main->addWidget(tagLbl);

    contentLbl = new QLabel;
    contentLbl->setWordWrap(true);
    contentLbl->setStyleSheet("color:#dbe7ff; font-size:14px; padding: 0px 12px 8px 12px;");
    main->addWidget(contentLbl);

    // 评论区（Instagram风格）
    commentArea = new QWidget(this);
    commentArea->setStyleSheet("background: transparent; padding: 0px 12px 12px 12px;");
    QVBoxLayout *commentLayout = new QVBoxLayout(commentArea);
    commentLayout->setContentsMargins(0, 0, 0, 0);
    commentLayout->setSpacing(8);
    commentArea->hide(); // 初始隐藏，有评论时显示
    main->addWidget(commentArea);

    // 评论输入框
    QHBoxLayout *inputLayout = new QHBoxLayout;
    inputLayout->setContentsMargins(12, 0, 12, 12);
    inputLayout->setSpacing(8);

    commentInput = new QLineEdit(this);
    commentInput->setPlaceholderText("Add a comment...");
    commentInput->setStyleSheet(
        "QLineEdit {"
        "  background: rgba(108,173,255,0.1);"
        "  border: 1px solid #6CADFF;"
        "  border-radius: 20px;"
        "  padding: 8px 16px;"
        "  color: white;"
        "  font-size: 14px;"
        "}"
        "QLineEdit:focus {"
        "  border: 1px solid #3A7DFF;"
        "  background: rgba(108,173,255,0.15);"
        "}"
    );
    commentInput->hide(); // 初始隐藏

    QPushButton *postBtn = new QPushButton("Post", this);
    postBtn->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #6CADFF, stop:1 #3A7DFF);"
        "  color: white;"
        "  border: none;"
        "  border-radius: 20px;"
        "  padding: 8px 20px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #7db8ff, stop:1 #4a8dff);"
        "}"
    );
    postBtn->hide(); // 初始隐藏

    inputLayout->addWidget(commentInput, 1);
    inputLayout->addWidget(postBtn);
    main->addLayout(inputLayout);

    connect(commentBtn, &QPushButton::clicked, this, [this, postBtn]() {
        commentInput->show();
        postBtn->show();
        commentInput->setFocus();
    });

    connect(postBtn, &QPushButton::clicked, this, [this, postBtn]() {
        QString text = commentInput->text().trimmed();
        if (!text.isEmpty()) {
            addComment(text);
            commentInput->clear();
        }
        commentInput->hide();
        postBtn->hide();
    });

    connect(commentInput, &QLineEdit::returnPressed, this, [this, postBtn]() {
        QString text = commentInput->text().trimmed();
        if (!text.isEmpty()) {
            addComment(text);
            commentInput->clear();
        }
        commentInput->hide();
        postBtn->hide();
    });

    updateCountDisplay();

    auto &langMgr = LanguageManager::instance();
    applyLanguage(langMgr.currentLanguage());
    connect(&langMgr, &LanguageManager::languageChanged, this, &FriendItem::applyLanguage);
    
    // 如果widget已经有父widget（已添加到布局），立即设置初始缩略图尺寸
    // 这样可以避免第一个item显示过大
    if (parent && thumbLbl) {
        // 使用多个延迟触发，确保布局已经完成
        QTimer::singleShot(50, this, [this]() {
            if (thumbLbl && width() > 0) {
                updateThumbnailSize();
            }
        });
        QTimer::singleShot(200, this, [this]() {
            if (thumbLbl && width() > 0) {
                updateThumbnailSize();
            }
        });
    }
}

FriendItem* FriendItem::fromPublish(const QString &videoThumb,
                                    const FriendPostCopy &copy)
{
    return new FriendItem("", "Me", videoThumb, QDateTime::currentDateTime(), -1, copy);
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

void FriendItem::onRepost()
{
    reposted = !reposted;
    repostCount += reposted ? 1 : -1;
    updateCountDisplay();
}

void FriendItem::onComment()
{
    // 点击comment按钮时，只显示输入框，不增加计数
    // 计数只在真正提交评论时增加（在addComment中）
    emit commentRequested(this);
}

void FriendItem::addComment(const QString &text)
{
    // 只在真正添加评论时增加计数（修复重复计数问题）
    if (!text.trimmed().isEmpty()) {
        commentCount++;
        updateCountDisplay();

        // 显示评论区
        commentArea->show();

        // 创建评论项（头像 + 昵称 + 评论内容）
        QWidget *commentItem = new QWidget(commentArea);
        commentItem->setStyleSheet("background: transparent;");
        QHBoxLayout *itemLayout = new QHBoxLayout(commentItem);
        itemLayout->setContentsMargins(0, 0, 0, 0);
        itemLayout->setSpacing(10);

        // 评论者头像（小圆形头像）
        QLabel *commentAvatar = new QLabel(commentItem);
        commentAvatar->setFixedSize(32, 32);
        commentAvatar->setScaledContents(false);
        
        QString commenterName = "You"; // 可以改为当前用户
        QString commenterAvatarPath = getAvatarPathForUser(commenterName);
        QPixmap commentAvatarPix = roundedFromPath(commenterAvatarPath, QSize(32, 32), 16);
        commentAvatar->setPixmap(commentAvatarPix);
        commentAvatar->setStyleSheet(
            "border: 1px solid #6CADFF;"
            "border-radius: 16px;"
            "background-color: transparent;"
        );

        // 评论内容
        QLabel *commentText = new QLabel(commentItem);
        commentText->setWordWrap(true);
        commentText->setText(QString("<b>%1</b> %2").arg(commenterName).arg(text));
        commentText->setStyleSheet("color: white; font-size: 14px; background: transparent;");

        itemLayout->addWidget(commentAvatar);
        itemLayout->addWidget(commentText, 1);
        commentArea->layout()->addWidget(commentItem);
    }
}

void FriendItem::updateCountDisplay()
{
    likeBtn->setText(QString(" %1").arg(likeCount));
    commentBtn->setText(QString(" %1").arg(commentCount));
    shareBtn->setText(QString(" %1").arg(shareCount));
    repostBtn->setText(QString(" %1").arg(repostCount));
    
    // 根据状态改变图标颜色（使用色环中的粉色作为激活色）
    QString baseStyle = 
        "QPushButton {"
        "  background: transparent;"
        "  color: white;"
        "  border: none;"
        "  padding: 8px 12px;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "  text-align: left;"
        "}"
        "QPushButton:hover {"
        "  background: rgba(108,173,255,0.1);"
        "  border-radius: 4px;"
        "}";
    
    QString activeStyle = 
        "QPushButton {"
        "  background: transparent;"
        "  color: #FF4F70;"
        "  border: none;"
        "  padding: 8px 12px;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "  text-align: left;"
        "}"
        "QPushButton:hover {"
        "  background: rgba(255,79,112,0.1);"
        "  border-radius: 4px;"
        "}";
    
    likeBtn->setStyleSheet(liked ? activeStyle : baseStyle);
    shareBtn->setStyleSheet(shared ? activeStyle : baseStyle);
    repostBtn->setStyleSheet(reposted ? activeStyle : baseStyle);
}

void FriendItem::setThumbnail(const QPixmap &pixmap)
{
    if (!pixmap.isNull()) {
        // 保存原始pixmap，用于后续缩放
        originalThumbPixmap_ = pixmap;
        thumbLbl->setStyleSheet("background: #0D0D0D;");
        thumbLbl->setText("");
        // 重置缓存尺寸，强制重新缩放
        lastThumbWidth_ = -1;
        lastThumbHeight_ = -1;
        // 直接调用resizeEvent逻辑来设置初始尺寸
        if (thumbLbl) {
            updateThumbnailSize();
        }
    }
}

void FriendItem::applyLanguage(AppLanguage lang)
{
    const QString fallbackTagZh = "#日常瞬间";
    const QString fallbackTagEn = "#DailyMoments";
    const QString fallbackCaptionZh = "分享一个新视频，欢迎来聊！";
    const QString fallbackCaptionEn = "Just dropped a new clip, come say hi!";

    const QString tagText = (lang == AppLanguage::Chinese ? copy_.tagZh : copy_.tagEn);
    const QString captionText = (lang == AppLanguage::Chinese ? copy_.captionZh : copy_.captionEn);

    tagLbl->setText(tagText.isEmpty()
                        ? (lang == AppLanguage::Chinese ? fallbackTagZh : fallbackTagEn)
                        : tagText);
    contentLbl->setText(captionText.isEmpty()
                            ? (lang == AppLanguage::Chinese ? fallbackCaptionZh : fallbackCaptionEn)
                            : captionText);
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
        else if (watched == thumbLbl && videoIndex_ >= 0)
        {
            emit thumbnailClicked(videoIndex_);
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void FriendItem::updateThumbnailSize()
{
    if (!thumbLbl) {
        return;
    }
    
    // 根据窗口宽度动态调整缩略图大小，严格保持16:9比例
    const int widgetWidth = width();
    
    // 如果widget宽度为0或无效，跳过更新（可能还在布局中）
    if (widgetWidth <= 0) {
        return;
    }
    
    const int padding = 0; // 左右padding已经在layout中处理
    const int availableWidth = widgetWidth - padding;
    
    // 如果可用宽度无效，跳过更新
    if (availableWidth <= 0) {
        return;
    }
    
    // 计算高度，严格保持16:9宽高比（不限制高度范围）
    const double aspectRatio = 16.0 / 9.0;
    int targetHeight = static_cast<int>(availableWidth / aspectRatio);
    
    // 性能优化：如果尺寸没有变化，跳过缩放
    if (lastThumbWidth_ == availableWidth && lastThumbHeight_ == targetHeight) {
        return;
    }
    
    lastThumbWidth_ = availableWidth;
    lastThumbHeight_ = targetHeight;
    
    // 更新缩略图大小 - 同时设置宽度和高度，确保比例正确
    thumbLbl->setFixedSize(availableWidth, targetHeight);
    
    // 优先使用缓存的原始pixmap
    if (!originalThumbPixmap_.isNull()) {
        // 使用 KeepAspectRatio 保持比例，并居中裁剪
        QPixmap scaledPixmap = originalThumbPixmap_.scaled(availableWidth, targetHeight, 
                                                          Qt::KeepAspectRatioByExpanding, 
                                                          Qt::SmoothTransformation);
        
        // 如果缩放后的图片比目标尺寸大，需要居中裁剪
        if (scaledPixmap.width() > availableWidth || scaledPixmap.height() > targetHeight) {
            int x = (scaledPixmap.width() - availableWidth) / 2;
            int y = (scaledPixmap.height() - targetHeight) / 2;
            scaledPixmap = scaledPixmap.copy(x, y, availableWidth, targetHeight);
        }
        
        thumbLbl->setPixmap(scaledPixmap);
    } else if (!thumbPath.isEmpty() && QFile::exists(thumbPath)) {
        // 如果没有缓存的pixmap，从文件加载
        QPixmap pixmap(thumbPath);
        if (!pixmap.isNull()) {
            originalThumbPixmap_ = pixmap;  // 缓存原始图片
            
            // 使用 KeepAspectRatio 保持比例，并居中裁剪
            QPixmap scaledPixmap = pixmap.scaled(availableWidth, targetHeight, 
                                                 Qt::KeepAspectRatioByExpanding, 
                                                 Qt::SmoothTransformation);
            
            // 如果缩放后的图片比目标尺寸大，需要居中裁剪
            if (scaledPixmap.width() > availableWidth || scaledPixmap.height() > targetHeight) {
                int x = (scaledPixmap.width() - availableWidth) / 2;
                int y = (scaledPixmap.height() - targetHeight) / 2;
                scaledPixmap = scaledPixmap.copy(x, y, availableWidth, targetHeight);
            }
            
            thumbLbl->setPixmap(scaledPixmap);
        }
    }
}

void FriendItem::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // 使用防抖机制：延迟更新，避免频繁缩放
    // 增加延迟时间，避免在布局调整时频繁触发
    static QTimer *debounceTimer = nullptr;
    if (!debounceTimer) {
        debounceTimer = new QTimer(this);
        debounceTimer->setSingleShot(true);
        debounceTimer->setInterval(150);  // 150ms防抖
        connect(debounceTimer, &QTimer::timeout, this, &FriendItem::updateThumbnailSize);
    }
    debounceTimer->stop();
    debounceTimer->start();
}

void FriendItem::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    // 当控件显示时，更新缩略图尺寸
    updateThumbnailSize();
}

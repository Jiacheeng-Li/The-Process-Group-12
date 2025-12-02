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
#include <QBitmap>
#include <QImageReader>
#include <QImage>
#include "../shared/narration_manager.h"

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
        // 使用 QImageReader 确保正确加载 JPG/PNG 等格式
        QImageReader reader(imagePath);
        QImage image = reader.read();
        if (!image.isNull()) {
            source = QPixmap::fromImage(image);
        } else {
            // 如果 QImageReader 失败，尝试直接加载
            source = QPixmap(imagePath);
        }
    }

    if (!source.isNull()) {
        QPixmap pix = source.scaled(size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QPainter painter(&base);
        painter.setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        // 对于圆形头像，使用椭圆路径
        if (radius == size.width() / 2 && size.width() == size.height()) {
            path.addEllipse(QRectF(0, 0, size.width(), size.height()));
        } else {
            path.addRoundedRect(QRectF(0, 0, size.width(), size.height()), radius, radius);
        }
        painter.setClipPath(path);
        painter.drawPixmap(0, 0, pix);
        painter.end();
    } else {
        // 使用色环配色方案生成渐变背景
        QPainter painter(&base);
        painter.setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        if (radius == size.width() / 2 && size.width() == size.height()) {
            path.addEllipse(QRectF(0, 0, size.width(), size.height()));
        } else {
            path.addRoundedRect(QRectF(0, 0, size.width(), size.height()), radius, radius);
        }
        QRadialGradient grad(size.width()/2, size.height()/2, size.width()/2);
        grad.setColorAt(0.0, QColor("#FF4F70"));
        grad.setColorAt(0.3, QColor("#FF8AA0"));
        grad.setColorAt(0.5, QColor("#6CADFF"));
        grad.setColorAt(0.7, QColor("#3A7DFF"));
        grad.setColorAt(1.0, QColor("#BFBFBF"));
        painter.fillPath(path, QBrush(grad));
        painter.end();
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
    setAttribute(Qt::WA_StyledBackground, true);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->setContentsMargins(0, 0, 0, 0);
    main->setSpacing(0);

    // 顶部：头像 + 用户名 + 时间
    QHBoxLayout *top = new QHBoxLayout;
    top->setContentsMargins(12, 12, 12, 8);
    top->setSpacing(10);

    // 圆形头像 - 使用mask确保真正圆形
    avatar = new QLabel(this);
    avatar->setObjectName("friendAvatar");
    avatar->setFixedSize(40, 40);
    avatar->setScaledContents(false);
    
    QString finalAvatarPath = avatarPath;
    if (finalAvatarPath.isEmpty() || !QFile::exists(finalAvatarPath)) {
        finalAvatarPath = getAvatarPathForUser(username);
    }
    
    // 使用Profile页相同的逻辑，确保圆形（radius = size/2）
    QPixmap avatarPix = roundedFromPath(finalAvatarPath, QSize(40, 40), 20);
    
    // 创建圆形mask确保头像真正是圆形
    QBitmap mask(40, 40);
    mask.fill(Qt::color0);
    QPainter maskPainter(&mask);
    maskPainter.setRenderHint(QPainter::Antialiasing);
    maskPainter.setBrush(Qt::color1);
    maskPainter.drawEllipse(0, 0, 40, 40);
    maskPainter.end();
    avatarPix.setMask(mask);
    
    avatar->setPixmap(avatarPix);
    // 确保 QLabel 是方形的，mask 会使其显示为圆形
    avatar->setScaledContents(false);
    avatar->setCursor(Qt::PointingHandCursor);
    avatar->installEventFilter(this);

    usernameLbl = new QLabel(username);
    usernameLbl->setObjectName("friendUsername");
    usernameLbl->setStyleSheet(
        "font-weight: 600;"
        "font-size: 15px;"
        "background: transparent;"
    );
    usernameLbl->setCursor(Qt::PointingHandCursor);
    usernameLbl->installEventFilter(this);

    timeLbl = new QLabel(time.toString("HH:mm"));
    timeLbl->setObjectName("friendTime");
    timeLbl->setStyleSheet(
        "font-size: 13px;"
        "font-weight: 500;"
        "background: transparent;"
    );

    top->addWidget(avatar);
    top->addWidget(usernameLbl);
    top->addStretch();
    top->addWidget(timeLbl);
    main->addLayout(top);

    // 视频缩略图（Instagram风格，响应式，保持16:9比例）
    thumbLbl = new QLabel;
    thumbLbl->setObjectName("friendVideoThumb");
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

    // Instagram风格的按钮样式，使用色环配色，放大字体
    QString buttonStyle = 
        "QPushButton {"
        "  background: transparent;"
        "  color: #e8f0ff;"
        "  border: none;"
        "  padding: 10px 14px;"
        "  font-size: 16px;"
        "  font-weight: 600;"
        "  text-align: left;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 rgba(108,173,255,0.15), "
        "    stop:1 rgba(255,138,160,0.12));"
        "  border-radius: 6px;"
        "  color: #ffffff;"
        "}";

    likeBtn = new QPushButton(this);
    likeBtn->setIcon(QIcon(":/icons/icons/like.svg"));
    likeBtn->setIconSize(QSize(32, 32)); // 再放大一些
    likeBtn->setText(" 0");
    likeBtn->setStyleSheet(buttonStyle);
    likeBtn->setCursor(Qt::PointingHandCursor);

    commentBtn = new QPushButton(this);
    commentBtn->setIcon(QIcon(":/icons/icons/comment.svg"));
    commentBtn->setIconSize(QSize(32, 32)); // 再放大一些
    commentBtn->setText(" 0");
    commentBtn->setStyleSheet(buttonStyle);
    commentBtn->setCursor(Qt::PointingHandCursor);

    shareBtn = new QPushButton(this);
    shareBtn->setIcon(QIcon(":/icons/icons/share.svg"));
    shareBtn->setIconSize(QSize(32, 32)); // 再放大一些
    shareBtn->setText(" 0");
    shareBtn->setStyleSheet(buttonStyle);
    shareBtn->setCursor(Qt::PointingHandCursor);

    repostBtn = new QPushButton(this);
    repostBtn->setIcon(QIcon(":/icons/icons/repost.svg"));
    repostBtn->setIconSize(QSize(32, 32)); // 再放大一些
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
    tagLbl->setObjectName("friendTag");
    tagLbl->setStyleSheet(
        "font-size: 14px;"
        "font-weight: 600;"
        "padding: 4px 12px;"
        "background: transparent;"
    );
    main->addWidget(tagLbl);

    contentLbl = new QLabel;
    contentLbl->setObjectName("friendCaption");
    contentLbl->setWordWrap(true);
    contentLbl->setStyleSheet(
        "font-size: 14px;"
        "font-weight: 400;"
        "line-height: 1.5;"
        "padding: 0px 12px 8px 12px;"
        "background: transparent;"
    );
    main->addWidget(contentLbl);

    // 评论区（Instagram风格）
    commentArea = new QWidget(this);
    commentArea->setObjectName("friendComments");
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
    commentInput->setObjectName("friendCommentInput");
    commentInput->setPlaceholderText("Add a comment...");
    commentInput->setStyleSheet(
        "QLineEdit {"
        "  border-radius: 20px;"
        "  padding: 8px 16px;"
        "  font-size: 14px;"
        "}"
    );
    commentInput->hide(); // 初始隐藏

    QPushButton *postBtn = new QPushButton("Post", this);
    postBtn->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 #3A7DFF, "
        "    stop:0.5 #6CADFF, "
        "    stop:1 #3A7DFF);"
        "  color: white;"
        "  border: 2px solid rgba(108,173,255,0.8);"
        "  border-radius: 20px;"
        "  padding: 8px 20px;"
        "  font-weight: 600;"
        "  font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 #6CADFF, "
        "    stop:0.5 #3A7DFF, "
        "    stop:1 #6CADFF);"
        "  border-color: rgba(108,173,255,1.0);"
        "}"
        "QPushButton:pressed {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 #2A6DEF, "
        "    stop:0.5 #5C9DFF, "
        "    stop:1 #2A6DEF);"
        "}"
    );
    postBtn->setCursor(Qt::PointingHandCursor);
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

    applyThemeStyles();

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
    // 语音播报
    NarrationManager::instance().narrate(
        liked ? QString::fromUtf8("已点赞") : QString::fromUtf8("取消点赞"),
        liked ? "Liked" : "Unliked"
    );
}

void FriendItem::onShare()
{
    shared = !shared;
    shareCount += shared ? 1 : -1;
    updateCountDisplay();
    // 语音播报
    NarrationManager::instance().narrate(
        shared ? QString::fromUtf8("已分享") : QString::fromUtf8("取消分享"),
        shared ? "Shared" : "Unshared"
    );
}

void FriendItem::onRepost()
{
    reposted = !reposted;
    repostCount += reposted ? 1 : -1;
    updateCountDisplay();
    // 语音播报
    NarrationManager::instance().narrate(
        reposted ? QString::fromUtf8("已转发") : QString::fromUtf8("取消转发"),
        reposted ? "Reposted" : "Unreposted"
    );
}

void FriendItem::onComment()
{
    // 点击comment按钮时，只显示输入框，不增加计数
    // 计数只在真正提交评论时增加（在addComment中）
    emit commentRequested(this);
}

void FriendItem::addComment(const QString &text, const QString &commenter)
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
        
        QString commenterName = commenter.isEmpty() ? "You" : commenter;
        QString commenterAvatarPath = getAvatarPathForUser(commenterName);
        QPixmap commentAvatarPix = roundedFromPath(commenterAvatarPath, QSize(32, 32), 16);
        
        // 创建圆形mask确保头像真正是圆形
        QBitmap mask(32, 32);
        mask.fill(Qt::color0);
        QPainter maskPainter(&mask);
        maskPainter.setRenderHint(QPainter::Antialiasing);
        maskPainter.setBrush(Qt::color1);
        maskPainter.drawEllipse(0, 0, 32, 32);
        maskPainter.end();
        commentAvatarPix.setMask(mask);
        
        commentAvatar->setPixmap(commentAvatarPix);
        commentAvatar->setProperty("isCommentAvatar", true);
        commentAvatar->setStyleSheet(
            "border-radius: 16px;"
            "background-color: transparent;"
        );

        // 评论内容
        QLabel *commentText = new QLabel(commentItem);
        commentText->setWordWrap(true);
        commentText->setProperty("commenterName", commenterName);
        commentText->setProperty("commentBody", text);
        restyleCommentLabel(commentText);

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
    
    const QString baseTextColor = highContrast_ ? "#f4c430" : (dayMode_ ? "#20324f" : "#e8f0ff");
    const QString hoverBg = dayMode_ ? "rgba(51,83,179,0.08)" : "rgba(108,173,255,0.15)";
    const QString hoverTextColor = dayMode_ ? "#0c1b33" : "#ffffff";
    const QString activeColor = dayMode_ ? "#d93f78" : "#FF4F70";
    const QString activeHoverBg = dayMode_ ? "rgba(217,63,120,0.12)" : "rgba(255,79,112,0.15)";
    
    QString baseStyle =
        QString("QPushButton {"
                "  background: transparent;"
                "  color: %1;"
                "  border: none;"
                "  padding: 10px 14px;"
                "  font-size: 16px;"
                "  font-weight: 600;"
                "  text-align: left;"
                "}"
                "QPushButton:hover {"
                "  background: %2;"
                "  border-radius: 6px;"
                "  color: %3;"
                "}")
            .arg(baseTextColor, hoverBg, hoverTextColor);

    QString activeStyle =
        QString("QPushButton {"
                "  background: transparent;"
                "  color: %1;"
                "  border: none;"
                "  padding: 10px 14px;"
                "  font-size: 16px;"
                "  font-weight: 600;"
                "  text-align: left;"
                "}"
                "QPushButton:hover {"
                "  background: %2;"
                "  border-radius: 6px;"
                "  color: %1;"
                "}")
            .arg(activeColor, activeHoverBg);
    
    // 更新按钮样式和图标（夜间/高对比使用 *1.svg，日间使用默认）
    const bool useAltIcons = highContrast_ || !dayMode_;
    if (likeBtn) {
        likeBtn->setStyleSheet(liked ? activeStyle : baseStyle);
        likeBtn->setIcon(QIcon(useAltIcons ? QStringLiteral(":/icons/icons/like1.svg")
                                           : QStringLiteral(":/icons/icons/like.svg")));
    }
    if (commentBtn) {
        commentBtn->setStyleSheet(baseStyle);
        commentBtn->setIcon(QIcon(useAltIcons ? QStringLiteral(":/icons/icons/comment1.svg")
                                              : QStringLiteral(":/icons/icons/comment.svg")));
    }
    if (shareBtn) {
        shareBtn->setStyleSheet(shared ? activeStyle : baseStyle);
        shareBtn->setIcon(QIcon(useAltIcons ? QStringLiteral(":/icons/icons/share1.svg")
                                            : QStringLiteral(":/icons/icons/share.svg")));
    }
    if (repostBtn) {
        repostBtn->setStyleSheet(reposted ? activeStyle : baseStyle);
        repostBtn->setIcon(QIcon(useAltIcons ? QStringLiteral(":/icons/icons/repost1.svg")
                                             : QStringLiteral(":/icons/icons/repost.svg")));
    }
}

void FriendItem::applyThemeStyles()
{
    QString cardBackground;
    QString borderColor;
    if (highContrast_) {
        cardBackground = QStringLiteral("#000000");
        borderColor = QStringLiteral("#f4c430");
    } else if (dayMode_) {
        cardBackground = QStringLiteral(
            "qlineargradient(x1:0, y1:0, x2:1, y2:1,"
            " stop:0 rgba(255,255,255,0.96),"
            " stop:0.35 rgba(235,242,255,0.94),"
            " stop:1 rgba(223,235,255,0.90))");
        borderColor = QStringLiteral("rgba(58,82,132,0.3)");
    } else {
        cardBackground = QStringLiteral(
            "qlineargradient(x1:0, y1:0, x2:1, y2:1,"
            " stop:0 rgba(8,20,60,0.90),"
            " stop:0.3 rgba(58,125,255,0.15),"
            " stop:0.6 rgba(108,173,255,0.12),"
            " stop:1 rgba(12,40,118,0.88))");
        borderColor = QStringLiteral("rgba(108,173,255,0.5)");
    }

    setStyleSheet(QStringLiteral(
        "QWidget {"
        "  background: %1;"
        "  border: 2px solid %2;"
        "  border-radius: 16px;"
        "  padding: 0px;"
        "  margin: 8px 0px;"
        "}"
    ).arg(cardBackground, borderColor));

    const QString avatarBorder = highContrast_ ? "#f4c430" : "#6CADFF";
    if (avatar) {
        avatar->setStyleSheet(QStringLiteral(
            "QLabel#friendAvatar {"
            "  border: 2px solid %1;"
            "  border-radius: 20px;"
            "  background-color: transparent;"
            "}"
            "QLabel#friendAvatar:hover {"
            "  border: 3px solid #FF8AA0;"
            "}"
        ).arg(avatarBorder));
    }

    const QString usernameColor = highContrast_ ? "#f4c430" : (dayMode_ ? "#0c1b33" : "#ffffff");
    const QString usernameHover = highContrast_ ? "#ffffff" : (dayMode_ ? "#3A7DFF" : "#6CADFF");
    if (usernameLbl) {
        usernameLbl->setStyleSheet(QStringLiteral(
            "QLabel#friendUsername {"
            "  font-weight: 600;"
            "  font-size: 15px;"
            "  color: %1;"
            "  background: transparent;"
            "}"
            "QLabel#friendUsername:hover {"
            "  color: %2;"
            "}"
        ).arg(usernameColor, usernameHover));
    }

    const QString timeColor = highContrast_ ? "#f4c430" : (dayMode_ ? "#5f6d8c" : "#a8c5ff");
    if (timeLbl) {
        timeLbl->setStyleSheet(QStringLiteral(
            "QLabel#friendTime {"
            "  font-size: 13px;"
            "  font-weight: 500;"
            "  color: %1;"
            "  background: transparent;"
            "}"
        ).arg(timeColor));
    }

    const QString tagColor = highContrast_ ? "#f4c430" : (dayMode_ ? "#3353b3" : "#6CADFF");
    if (tagLbl) {
        tagLbl->setStyleSheet(QStringLiteral(
            "QLabel#friendTag {"
            "  color: %1;"
            "  font-size: %2px;"
            "  font-weight: 600;"
            "  padding: 4px 12px;"
            "  background: transparent;"
            "}"
        ).arg(tagColor,
              highContrast_ ? QStringLiteral("16") : QStringLiteral("14")));
    }

    // 高对比模式下，正文也使用黄色以获得最大对比度
    const QString contentColor = highContrast_ ? "#f4c430" : (dayMode_ ? "#2d3a4f" : "#e8f0ff");
    if (contentLbl) {
        contentLbl->setStyleSheet(QStringLiteral(
            "QLabel#friendCaption {"
            "  color: %1;"
            "  font-size: %2px;"
            "  font-weight: 400;"
            "  line-height: 1.5;"
            "  padding: 0px 12px 8px 12px;"
            "  background: transparent;"
            "}"
        ).arg(contentColor,
              highContrast_ ? QStringLiteral("16") : QStringLiteral("14")));
    }

    const QString thumbBg = highContrast_ ? "#000000" : (dayMode_ ? "#f6f8ff" : "#0D0D0D");
    const QString thumbTextColor = highContrast_ ? "#f4c430" : (dayMode_ ? "#20324f" : "#ffffff");
    if (thumbLbl) {
        thumbLbl->setStyleSheet(QStringLiteral(
            "QLabel#friendVideoThumb {"
            "  background: %1;"
            "  color: %2;"
            "  font-size: 48px;"
            "}"
        ).arg(thumbBg, thumbTextColor));
    }

    const QString inputBg = highContrast_ ? "rgba(244,196,48,0.12)" : (dayMode_ ? "rgba(51,83,179,0.08)" : "rgba(108,173,255,0.12)");
    const QString inputBorder = highContrast_ ? "#f4c430" : (dayMode_ ? "rgba(51,83,179,0.45)" : "#6CADFF");
    const QString inputFocusBg = highContrast_ ? "rgba(244,196,48,0.2)" : (dayMode_ ? "rgba(51,83,179,0.16)" : "rgba(108,173,255,0.2)");
    const QString inputColor = highContrast_ ? "#ffffff" : (dayMode_ ? "#0c1b33" : "#ffffff");
    const QString inputFocusBorder = highContrast_ ? "#ffd700" : (dayMode_ ? "#3353b3" : "#3A7DFF");
    if (commentInput) {
        commentInput->setStyleSheet(QStringLiteral(
            "QLineEdit#friendCommentInput {"
            "  background: %1;"
            "  border: 1px solid %2;"
            "  border-radius: 20px;"
            "  padding: 8px 16px;"
            "  color: %4;"
            "  font-size: 14px;"
            "}"
            "QLineEdit#friendCommentInput:focus {"
            "  border: 1px solid %3;"
            "  background: %5;"
            "}"
        ).arg(inputBg, inputBorder, inputFocusBorder, inputColor, inputFocusBg));
    }

    updateCountDisplay();
    restyleCommentLabels();
}

void FriendItem::restyleCommentLabels()
{
    if (!commentArea) {
        return;
    }
    const auto labels = commentArea->findChildren<QLabel*>();
    const QString commentAvatarBorder = highContrast_ ? "#f4c430" : "#6CADFF";
    for (QLabel *label : labels) {
        if (!label) {
            continue;
        }
        if (label->property("commenterName").isValid()) {
            restyleCommentLabel(label);
        } else if (label->property("isCommentAvatar").isValid()) {
            label->setStyleSheet(QStringLiteral(
                "border: 2px solid %1;"
                "border-radius: 16px;"
                "background-color: transparent;"
            ).arg(commentAvatarBorder));
        }
    }
}

void FriendItem::restyleCommentLabel(QLabel *label)
{
    if (!label) {
        return;
    }
    const QString commenterName = label->property("commenterName").toString();
    const QString commentBody = label->property("commentBody").toString();
    const QString nameColor = highContrast_ ? "#f4c430" : (dayMode_ ? "#3353b3" : "#6CADFF");
    // 在高对比模式下，正文也使用黄色以获得最大对比度
    const QString bodyColor = highContrast_ ? "#f4c430" : (dayMode_ ? "#2d3a4f" : "#e8f0ff");
    const QString html = QStringLiteral(
        "<span style='font-weight:600; color:%1;'>%2</span> "
        "<span style='color:%3;'>%4</span>")
            .arg(nameColor,
                 commenterName.toHtmlEscaped(),
                 bodyColor,
                 commentBody.toHtmlEscaped());
    label->setText(html);
    label->setStyleSheet(QStringLiteral(
        "color: %1;"
        "font-size: %2px;"
        "background: transparent;"
        "line-height: 1.4;"
    ).arg(bodyColor,
          highContrast_ ? QStringLiteral("15") : QStringLiteral("14")));
}

void FriendItem::setDayMode(bool dayMode)
{
    if (dayMode_ == dayMode) {
        return;
    }
    dayMode_ = dayMode;
    applyThemeStyles();
}

void FriendItem::setHighContrastMode(bool enabled)
{
    if (highContrast_ == enabled) {
        return;
    }
    highContrast_ = enabled;
    // 高对比模式下不再使用日间模式的浅色背景
    if (highContrast_) {
        dayMode_ = false;
    }
    applyThemeStyles();
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
        if (watched == avatar || watched == usernameLbl)
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

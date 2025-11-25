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


// 根据用户名获取头像文件路径
static QString getAvatarPathForUser(const QString &username)
{
    // 前5个用户映射到头像文件 1-5.jpg
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

// 将图片裁剪成圆形（完全仿照Profile页的逻辑）
static QPixmap createRoundAvatar(const QString &imagePath, int size, const QString &username = "")
{
    QPixmap result(size, size);
    result.fill(Qt::transparent);

    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPainterPath circlePath;
    // 使用 addRoundedRect 当 radius = size/2 时就是圆形（与Profile页一致）
    int radius = size / 2;
    circlePath.addRoundedRect(QRectF(0, 0, size, size), radius, radius);
    painter.setClipPath(circlePath);

    QPixmap source;
    if (!imagePath.isEmpty() && QFile::exists(imagePath)) {
        source = QPixmap(imagePath);
    }

    if (!source.isNull()) {
        // 完全仿照Profile页：先缩放，然后直接绘制，让clipPath处理裁剪
        QPixmap scaled = source.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        // 直接绘制整个缩放后的pixmap，clipPath会自动裁剪成圆形
        painter.drawPixmap(0, 0, scaled);
    } else {
        // 纯色头像
        QStringList colors = {"#2f8dff", "#ff6b6b", "#4ecdc4", "#ffe66d", "#ff9ff3", "#54a0ff", "#5f27cd", "#00d2d3"};
        int colorIndex = username.isEmpty() ? 0 : qAbs(qHash(username)) % colors.size();
        painter.fillPath(circlePath, QColor(colors[colorIndex]));

        if (!username.isEmpty()) {
            painter.setPen(Qt::white);
            painter.setFont(QFont("Arial", size / 2.4, QFont::Bold));
            painter.drawText(QRect(0, 0, size, size), Qt::AlignCenter, username.left(1).toUpper());
        }
    }

    painter.end();
    return result;
}

FriendItem::FriendItem(const QString &avatarPath,
                       const QString &username,
                       const QString &videoThumb,
                       const QDateTime &time,
                       QWidget *parent)
    : QWidget(parent), thumbPath(videoThumb)
{
    setStyleSheet(
        "QWidget { "
        "  background: rgba(2,8,20,0.92); "
        "  border-bottom: 1px solid rgba(63,134,255,0.2); "
        "  border-radius: 12px; "
        "  padding: 12px; "
        "  margin: 4px 0px; "
        "}"
        "QLabel { color: white; }"
        "QPushButton { "
        "  background: transparent; "
        "  color: #8aa7d9; "
        "  border: none; "
        "  padding: 4px 8px; "
        "  border-radius: 6px; "
        "}"
        "QPushButton:hover { "
        "  background: rgba(63,134,255,0.15); "
        "  color: #9db6ff; "
        "}"
    );

    // 圆形头像（完全仿照Profile页的设置）
    avatar = new QLabel(this);
    avatar->setFixedSize(48, 48);
    avatar->setScaledContents(true);  // 与Profile页一致
    
    // 获取用户对应的头像路径
    // 如果传入的 avatarPath 参数不为空且文件存在，使用它；否则根据用户名查找
    QString finalAvatarPath = avatarPath;
    if (finalAvatarPath.isEmpty() || !QFile::exists(finalAvatarPath)) {
        finalAvatarPath = getAvatarPathForUser(username);
    }
    
    // 创建圆形头像（pixmap本身已经是圆形）
    QPixmap avatarPix = createRoundAvatar(finalAvatarPath, 48, username);
    avatar->setPixmap(avatarPix);
    // 不设置border-radius样式表，因为pixmap本身已经是圆形的（与Profile页一致）
    avatar->setStyleSheet(
        "border: 2px solid rgba(157,182,255,0.45);"
        "background-color: transparent;"
    );

    avatar->installEventFilter(this);

    usernameLbl = new QLabel(username);
    usernameLbl->setStyleSheet("font-weight:bold; font-size:15px; color: white;");

    timeLbl = new QLabel(time.toString("hh:mm\nyyyy.MM.dd"));
    timeLbl->setAlignment(Qt::AlignRight);
    timeLbl->setStyleSheet("color:#8aa7d9; font-size:12px;");

    QHBoxLayout *top = new QHBoxLayout;
    top->addWidget(avatar);
    top->addSpacing(6);
    top->addWidget(usernameLbl);
    top->addStretch();
    top->addWidget(timeLbl);

    thumbLbl = new QLabel;
    thumbLbl->setFixedSize(280, 420);
    thumbLbl->setScaledContents(true);
    thumbLbl->setAlignment(Qt::AlignCenter);
    
    // 初始状态：如果没有传入缩略图路径，显示占位符
    // 实际的缩略图会通过 setThumbnail() 方法设置
    if (!videoThumb.isEmpty() && QFile::exists(videoThumb)) {
        QPixmap pixmap(videoThumb);
        if (!pixmap.isNull()) {
            thumbLbl->setPixmap(pixmap.scaled(280, 420, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
            thumbLbl->setStyleSheet("border-radius:10px;");
        } else {
            // 文件存在但无法加载，显示占位符
            thumbLbl->setText(QString::fromUtf8("📹"));
            thumbLbl->setStyleSheet("border-radius:10px; background: rgba(63,134,255,0.2); color: white; font-size: 48px;");
        }
    } else {
        // 没有缩略图路径，显示占位符
        thumbLbl->setText(QString::fromUtf8("📹"));
        thumbLbl->setStyleSheet("border-radius:10px; background: rgba(63,134,255,0.2); color: white; font-size: 48px;");
    }

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
    return new FriendItem("", "Me", videoThumb, QDateTime::currentDateTime());
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

void FriendItem::setThumbnail(const QPixmap &pixmap)
{
    if (!pixmap.isNull()) {
        thumbLbl->setPixmap(pixmap.scaled(280, 420, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        thumbLbl->setStyleSheet("border-radius:10px;");
        thumbLbl->setText("");
    }
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

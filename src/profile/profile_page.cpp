#include "profile_page.h"
#include "../shared/narration_manager.h"

#include <QButtonGroup>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QSizePolicy>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QConicalGradient>
#include <QPaintEvent>
#include <QAbstractSlider>
#include <QFile>
#include <QDir>
#include <QApplication>
#include <QPixmap>
#include <QBitmap>
#include <QHash>
#include <QImageReader>
#include <QImage>
#include <QDebug>
#include <memory>
#include <algorithm>

namespace {
} // namespace

// 从图标创建圆形头像
QPixmap roundedFromIcon(const QIcon *icon, const QSize &size, int radius) {
    QPixmap base(size);
    base.fill(Qt::transparent);

    if (icon) {
        QPixmap pix = icon->pixmap(size);
        pix = pix.scaled(size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
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

// 获取用户头像路径（与FriendItem中的函数相同）
static QString getAvatarPathForUser(const QString &username)
{
    // Map the first five demo users to avatar files 1-5.jpg
    QStringList avatarUsers = {"Alice", "Bob", "Ethan", "Luna", "Olivia"};
    int index = avatarUsers.indexOf(username);
    
    if (index >= 0 && index < 5) {
        QString fileName = QString::number(index + 1) + ".jpg";
        QString appDir = QApplication::applicationDirPath();
        QStringList searchPaths = {
            QDir::currentPath() + "/friends/avatar/" + fileName,
            QDir::currentPath() + "/../friends/avatar/" + fileName,
            QDir::currentPath() + "/src/friends/avatar/" + fileName,
            QDir::currentPath() + "/../src/friends/avatar/" + fileName,
            QDir::currentPath() + "/../../src/friends/avatar/" + fileName,
            appDir + "/friends/avatar/" + fileName,
            appDir + "/../friends/avatar/" + fileName,
            appDir + "/../../friends/avatar/" + fileName,
            appDir + "/../../src/friends/avatar/" + fileName,
            appDir + "/../../../src/friends/avatar/" + fileName,
            "friends/avatar/" + fileName,
            "../friends/avatar/" + fileName,
            "src/friends/avatar/" + fileName
        };
        
        for (const QString &path : searchPaths) {
            QString normalizedPath = QDir::cleanPath(path);
            if (QFile::exists(normalizedPath)) {
                qDebug() << "[ProfilePage] 找到头像文件:" << normalizedPath << "for user" << username;
                return normalizedPath;
            }
        }
        
        qDebug() << "[ProfilePage] 未找到头像文件 for user" << username << "fileName:" << fileName;
        qDebug() << "[ProfilePage] 当前目录:" << QDir::currentPath();
        qDebug() << "[ProfilePage] 应用目录:" << appDir;
    }
    
    return "";
}

// 从路径创建圆形头像
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
        if (radius == size.width() / 2 && size.width() == size.height()) {
            path.addEllipse(QRectF(0, 0, size.width(), size.height()));
        } else {
            path.addRoundedRect(QRectF(0, 0, size.width(), size.height()), radius, radius);
        }
        painter.setClipPath(path);
        painter.drawPixmap(0, 0, pix);
        painter.end();
    } else {
        base.fill(QColor("#2d2d2d"));
    }

    return base;
}

// 彩色头像环（仿 Instagram），使用项目配色做渐变
class AvatarRingWidget : public QWidget {
public:
    explicit AvatarRingWidget(const QIcon *icon, QWidget *parent = nullptr)
        : QWidget(parent) {
        if (icon) {
            icon_.reset(new QIcon(*icon));
        }
        setFixedSize(120, 120);
        setAttribute(Qt::WA_TranslucentBackground, true);
    }
    
    void setAvatarFromPath(const QString &avatarPath) {
        avatarPath_ = avatarPath;
        icon_.reset(); // 清除icon，使用路径
        update(); // 触发重绘
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QRectF outerRect = rect().adjusted(1, 1, -1, -1);

        // 外圈：多色渐变环
        QConicalGradient grad(outerRect.center(), 0);
        // 多段渐变，让颜色衔接更顺滑，尽量减少纯黑
        grad.setColorAt(0.00, QColor("#FF4F70"));   // 粉
        grad.setColorAt(0.18, QColor("#FF8AA0"));   // 粉 → 亮一点
        grad.setColorAt(0.32, QColor("#6CADFF"));   // 淡蓝
        grad.setColorAt(0.46, QColor("#3A7DFF"));   // 深蓝
        grad.setColorAt(0.60, QColor("#BFBFBF"));   // 银灰
        grad.setColorAt(0.74, QColor("#6CADFF"));   // 回到淡蓝
        grad.setColorAt(0.88, QColor("#FF8AA0"));   // 再次粉色过渡
        grad.setColorAt(1.00, QColor("#FF4F70"));   // 闭环粉

        painter.setPen(Qt::NoPen);
        painter.setBrush(grad);
        painter.drawEllipse(outerRect);

        // 内圈：深色背景 + 头像
        const int ringWidth = 5;  // 加粗色环
        QRectF innerRect = outerRect.adjusted(ringWidth, ringWidth, -ringWidth, -ringWidth);
        // 内圈背景改成深蓝，而不是纯黑
        painter.setBrush(QColor("#050b1e"));
        painter.drawEllipse(innerRect);

        if (icon_) {
            const int d = static_cast<int>(qMin(innerRect.width(), innerRect.height()));
            QSize avatarSize(d, d);
            QPixmap avatarPixmap = roundedFromIcon(icon_.get(), avatarSize, d / 2);

            QPointF topLeft(innerRect.center().x() - avatarPixmap.width() / 2.0,
                            innerRect.center().y() - avatarPixmap.height() / 2.0);
            painter.drawPixmap(topLeft, avatarPixmap);
        } else if (!avatarPath_.isEmpty()) {
            // 从路径加载头像
            const int d = static_cast<int>(qMin(innerRect.width(), innerRect.height()));
            QSize avatarSize(d, d);
            QPixmap avatarPixmap = roundedFromPath(avatarPath_, avatarSize, d / 2);

            QPointF topLeft(innerRect.center().x() - avatarPixmap.width() / 2.0,
                            innerRect.center().y() - avatarPixmap.height() / 2.0);
            painter.drawPixmap(topLeft, avatarPixmap);
        }
    }

private:
    std::unique_ptr<QIcon> icon_;
    QString avatarPath_;
};

QString defaultProfileStyle() {
    return QStringLiteral(
        "QWidget#profilePageRoot {"
        "  background: qradialgradient(cx:0.25, cy:0.2, radius:1.25,"
        "    stop:0 #091230, stop:0.5 #030918, stop:1 #00040a);"
        "}"
        "QScrollArea { background: transparent; border: none; }"
        "QScrollArea > QWidget > QWidget { background: transparent; }"
        "QFrame#contentShell {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "    stop:0 rgba(8,20,60,0.95), "
        "    stop:0.3 rgba(58,125,255,0.15), "
        "    stop:0.6 rgba(108,173,255,0.12), "
        "    stop:1 rgba(12,40,118,0.9));"
        "  border-radius: 48px;"
        "  border: 2px solid rgba(108,173,255,0.5);"
        "  outline: 1px solid rgba(255,79,112,0.2);"
        "}"
        "QFrame#heroCard {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "    stop:0 rgba(8,20,60,0.85), "
        "    stop:0.25 rgba(58,125,255,0.25), "
        "    stop:0.5 rgba(12,40,118,0.90), "
        "    stop:0.75 rgba(58,125,255,0.20), "
        "    stop:1 rgba(8,20,60,0.85));"
        "  border-radius: 32px;"
        "  border: 2px solid rgba(108,173,255,0.5);"
        "  outline: 1px solid rgba(255,79,112,0.3);"
        "}"
        "QHBoxLayout { background: transparent; }"
        "QVBoxLayout { background: transparent; }"
        "QLabel#displayName { font-size: 30px; font-weight: 700; color: #ffffff; }"
        "QLabel#username { font-size: 15px; color: #d0e4ff; font-weight: 600; }"
        "QLabel#bioLabel { color: #f0f5ff; line-height: 1.6; font-weight: 500; }"
        "QLabel#statValue { font-size: 20px; font-weight: 700; color: #ffffff; }"
        "QLabel#statLabel { font-size: 12px; text-transform: uppercase; color: #c8dcff; letter-spacing: 0.2em; font-weight: 600; }"
        "QPushButton#primaryCta {"
        "  background-color: #2f8dff;"
        "  border-radius: 22px;"
        "  padding: 12px 26px;"
        "  border: 2px solid rgba(76,162,255,0.8);"
        "  color: white;"
        "  font-weight: 700;"
        "  text-align: left;"
        "}"
        "QPushButton#primaryCta:hover {"
        "  background-color: #4ca2ff;"
        "  border-color: rgba(108,173,255,1.0);"
        "}"
        "QPushButton#primaryCta:checked {"
        "  background-color: rgba(22,37,66,0.85);"
        "  border: 2px solid rgba(76,162,255,0.8);"
        "  color: #9db6ff;"
        "}"
        "QPushButton#secondaryCta {"
        "  background-color: transparent;"
        "  color: white;"
        "  border: 1px solid rgba(76,162,255,0.5);"
        "  border-radius: 22px;"
        "  padding: 11px 24px;"
        "  font-weight: 600;"
        "  text-align: left;"
        "}"
        "QPushButton#secondaryCta:hover {"
        "  background-color: rgba(76,162,255,0.1);"
        "  border-color: rgba(76,162,255,0.8);"
        "}"
        "QFrame#videoFilter {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "    stop:0 rgba(8,20,60,0.80), "
        "    stop:0.33 rgba(58,125,255,0.25), "
        "    stop:0.66 rgba(12,40,118,0.85), "
        "    stop:1 rgba(8,20,60,0.80));"
        "  border-radius: 20px;"
        "  border: 2px solid rgba(108,173,255,0.5);"
        "  outline: 1px solid rgba(255,79,112,0.25);"
        "}"
        "QFrame#videoGrid {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "    stop:0 rgba(8,20,60,0.85), "
        "    stop:0.2 rgba(58,125,255,0.25), "
        "    stop:0.4 rgba(12,40,118,0.90), "
        "    stop:0.6 rgba(58,125,255,0.20), "
        "    stop:0.8 rgba(12,40,118,0.85), "
        "    stop:1 rgba(8,20,60,0.85));"
        "  border-radius: 30px;"
        "  border: 2px solid rgba(108,173,255,0.5);"
        "  outline: 1px solid rgba(255,79,112,0.3);"
        "  padding: 6px;"
        "}"
        "QPushButton#filterTab {"
        "  background-color: transparent;"
        "  color: #6f84a8;"
        "  border: none;"
        "  font-weight: 600;"
        "  padding: 8px 16px;"
        "  border-radius: 14px;"
        "}"
        "QPushButton#filterTab:hover { "
        "  color: #d3ddff;"
        "  background-color: rgba(108,173,255,0.15);"
        "}"
        "QPushButton#filterTab:checked {"
        "  color: white;"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 rgba(58,125,255,0.35), "
        "    stop:0.5 rgba(108,173,255,0.40), "
        "    stop:1 rgba(58,125,255,0.35));"
        "  border: 1px solid rgba(108,173,255,0.6);"
        "}"
        "QPushButton#videoTile {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "    stop:0 rgba(8,20,60,0.85), "
        "    stop:0.5 rgba(58,125,255,0.25), "
        "    stop:1 rgba(12,40,118,0.90));"
        "  border: 2px solid rgba(108,173,255,0.4);"
        "  border-radius: 20px;"
        "  min-height: 260px;"
        "}"
        "QPushButton#videoTile:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "    stop:0 rgba(58,125,255,0.35), "
        "    stop:0.5 rgba(108,173,255,0.40), "
        "    stop:1 rgba(255,138,160,0.30));"
        "  border: 2px solid rgba(108,173,255,0.7);"
        "  transform: scale(1.02);"
        "}"
        "QLabel#sectionTitle { font-size: 20px; font-weight: 700; color: #ffffff; }");
}

QString dayModeProfileStyle() {
    return QStringLiteral(
        "QWidget#profilePageRoot {"
        "  background: qradialgradient(cx:0.25, cy:0.2, radius:1.25,"
        "    stop:0 #e8f0ff, stop:0.5 #dde8f5, stop:1 #f4f7ff);"
        "}"
        "QScrollArea { background: transparent; border: none; }"
        "QScrollArea > QWidget > QWidget { background: transparent; }"
        "QFrame#contentShell {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "    stop:0 rgba(240,245,255,0.95), "
        "    stop:0.3 rgba(220,235,255,0.90), "
        "    stop:0.6 rgba(200,225,255,0.85), "
        "    stop:1 rgba(240,245,255,0.95));"
        "  border-radius: 48px;"
        "  border: 2px solid rgba(108,173,255,0.4);"
        "  outline: 1px solid rgba(255,79,112,0.2);"
        "}"
        "QFrame#heroCard {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "    stop:0 rgba(240,245,255,0.98), "
        "    stop:0.25 rgba(220,235,255,0.95), "
        "    stop:0.5 rgba(200,225,255,0.92), "
        "    stop:0.75 rgba(220,235,255,0.95), "
        "    stop:1 rgba(240,245,255,0.98));"
        "  border-radius: 32px;"
        "  border: 2px solid rgba(108,173,255,0.5);"
        "  outline: 1px solid rgba(255,79,112,0.25);"
        "}"
        "QHBoxLayout { background: transparent; }"
        "QVBoxLayout { background: transparent; }"
        "QLabel#displayName { font-size: 30px; font-weight: 700; color: #0c1b33; }"
        "QLabel#username { font-size: 15px; color: #4f5f7f; font-weight: 600; }"
        "QLabel#bioLabel { color: #2d3a4f; line-height: 1.6; font-weight: 500; }"
        "QLabel#statValue { font-size: 20px; font-weight: 700; color: #0c1b33; }"
        "QLabel#statLabel { font-size: 12px; text-transform: uppercase; color: #5f6d8c; letter-spacing: 0.2em; font-weight: 600; }"
        "QPushButton#primaryCta {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 #3A7DFF, "
        "    stop:0.5 #6CADFF, "
        "    stop:1 #3A7DFF);"
        "  border-radius: 22px;"
        "  padding: 12px 26px;"
        "  border: 2px solid rgba(108,173,255,0.8);"
        "  color: white;"
        "  font-weight: 700;"
        "  text-align: left;"
        "}"
        "QPushButton#primaryCta:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 #6CADFF, "
        "    stop:0.5 #3A7DFF, "
        "    stop:1 #6CADFF);"
        "  border-color: rgba(108,173,255,1.0);"
        "}"
        "QPushButton#primaryCta:checked {"
        "  background: rgba(200,220,255,0.85);"
        "  border: 2px solid rgba(76,162,255,0.8);"
        "  color: #1a3d7f;"
        "}"
        "QPushButton#secondaryCta {"
        "  background-color: transparent;"
        "  color: #2d3a4f;"
        "  border: 2px solid rgba(108,173,255,0.6);"
        "  border-radius: 22px;"
        "  padding: 11px 24px;"
        "  font-weight: 600;"
        "  text-align: left;"
        "}"
        "QPushButton#secondaryCta:hover {"
        "  background-color: rgba(108,173,255,0.15);"
        "  border-color: rgba(108,173,255,0.8);"
        "}"
        "QFrame#videoFilter {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "    stop:0 rgba(240,245,255,0.95), "
        "    stop:0.33 rgba(220,235,255,0.90), "
        "    stop:0.66 rgba(200,225,255,0.85), "
        "    stop:1 rgba(240,245,255,0.95));"
        "  border-radius: 20px;"
        "  border: 2px solid rgba(108,173,255,0.5);"
        "  outline: 1px solid rgba(255,79,112,0.25);"
        "}"
        "QFrame#videoGrid {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "    stop:0 rgba(240,245,255,0.95), "
        "    stop:0.2 rgba(220,235,255,0.90), "
        "    stop:0.4 rgba(200,225,255,0.85), "
        "    stop:0.6 rgba(220,235,255,0.90), "
        "    stop:0.8 rgba(200,225,255,0.85), "
        "    stop:1 rgba(240,245,255,0.95));"
        "  border-radius: 30px;"
        "  border: 2px solid rgba(108,173,255,0.5);"
        "  outline: 1px solid rgba(255,79,112,0.25);"
        "  padding: 6px;"
        "}"
        "QPushButton#filterTab {"
        "  background-color: transparent;"
        "  color: #5f6d8c;"
        "  border: none;"
        "  font-weight: 600;"
        "  padding: 8px 16px;"
        "  border-radius: 14px;"
        "}"
        "QPushButton#filterTab:hover { "
        "  color: #2d3a4f;"
        "  background-color: rgba(108,173,255,0.20);"
        "}"
        "QPushButton#filterTab:checked {"
        "  color: #0c1b33;"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "    stop:0 rgba(108,173,255,0.35), "
        "    stop:0.5 rgba(58,125,255,0.40), "
        "    stop:1 rgba(108,173,255,0.35));"
        "  border: 1px solid rgba(108,173,255,0.6);"
        "}"
        "QPushButton#videoTile {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "    stop:0 rgba(240,245,255,0.90), "
        "    stop:0.5 rgba(220,235,255,0.85), "
        "    stop:1 rgba(200,225,255,0.90));"
        "  border: 2px solid rgba(108,173,255,0.4);"
        "  border-radius: 20px;"
        "  min-height: 260px;"
        "}"
        "QPushButton#videoTile:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "    stop:0 rgba(108,173,255,0.25), "
        "    stop:0.5 rgba(58,125,255,0.30), "
        "    stop:1 rgba(108,173,255,0.25));"
        "  border: 2px solid rgba(108,173,255,0.7);"
        "}"
        "QLabel#sectionTitle { font-size: 20px; font-weight: 700; color: #0c1b33; }");
}

QString highContrastProfileStyle() {
    return QStringLiteral(
        "QWidget#profilePageRoot { background: #0b0b0b; }"
        "QScrollArea { background: transparent; border: none; }"
        "QFrame#contentShell {"
        "  background: #1b1b1b;"
        "  border-radius: 48px;"
        "  border: 2px solid #f4c430;"
        "}"
        "QFrame#heroCard {"
        "  background: #0f0f0f;"
        "  border-radius: 32px;"
        "  border: 2px solid #ff9f1c;"
        "}"
        "QLabel#displayName { font-size: 30px; font-weight: 700; color: #ffffff; }"
        "QLabel#username { font-size: 15px; color: #f9f871; }"
        "QLabel#bioLabel { color: #ffffff; }"
        "QLabel#statValue { font-size: 20px; font-weight: 700; color: #ffffff; }"
        "QLabel#statLabel { font-size: 12px; text-transform: uppercase; color: #f4c430; letter-spacing: 0.2em; }"
        "QPushButton#primaryCta {"
        "  background-color: #ff9f1c;"
        "  border-radius: 22px;"
        "  padding: 12px 26px;"
        "  border: none;"
        "  color: #000;"
        "  font-weight: 700;"
        "}"
        "QPushButton#secondaryCta {"
        "  background-color: transparent;"
        "  color: #ffffff;"
        "  border: 2px solid #f4c430;"
        "  border-radius: 22px;"
        "  padding: 11px 24px;"
        "  font-weight: 600;"
        "}"
        "QFrame#videoFilter {"
        "  background: #0f0f0f;"
        "  border-radius: 20px;"
        "  border: 2px solid #f4c430;"
        "}"
        "QFrame#videoGrid {"
        "  background: #0f0f0f;"
        "  border-radius: 30px;"
        "  border: 2px solid #f4c430;"
        "  padding: 6px;"
        "}"
        "QPushButton#filterTab {"
        "  background-color: transparent;"
        "  color: #f4c430;"
        "  border: none;"
        "  font-weight: 600;"
        "  padding: 8px 16px;"
        "  border-radius: 14px;"
        "}"
        "QPushButton#filterTab:checked {"
        "  color: #000;"
        "  background-color: #f4c430;"
        "}"
        "QPushButton#videoTile {"
        "  background-color: #141414;"
        "  border: 2px solid #f4c430;"
        "  border-radius: 20px;"
        "  min-height: 260px;"
        "}"
        "QLabel#sectionTitle { font-size: 20px; font-weight: 700; color: #f9f871; }");
}

struct ProfileCopy {
    QString displayName;
    QString username;
    QString bio;
    QString follow;
    QString following;
    QString share;
    QString replays;
    QString filterGrid;
    QString filterDrafts;
    QString filterTagged;
    QString emptyGrid;
    QString emptyDrafts;
    QString emptyTagged;
    QString statFollowing;
    QString statFollowers;
    QString statLikes;
};

ProfileCopy localizedProfileCopy(AppLanguage language) {
    if (language == AppLanguage::English) {
        return {
            "Lina Mendes",
            "@linagoesreal",
            "Creator of rooftop BeReal drops • chasing golden hour on every continent • new lens drops every Sunday.",
            "Follow",
            "Following",
            "Share profile",
            "Replays",
            "Grid",
            "Drafts",
            "Tagged",
            "No drops yet. Start sharing your world to fill this grid.",
            "Drafts you save will land here before they are published.",
            "When collaborators tag Lina, the replays will appear here.",
            "Following",
            "Followers",
            "Likes"};
    }

    return {
        QString::fromUtf8("莉娜·门德斯"),
        QString::fromUtf8("@linagoesreal"),
        QString::fromUtf8("屋顶双摄创作者 · 追逐每一座城市的黄金时刻 · 每周日更新新镜头。"),
        QString::fromUtf8("关注"),
        QString::fromUtf8("已关注"),
        QString::fromUtf8("分享主页"),
        QString::fromUtf8("回放"),
        QString::fromUtf8("网格"),
        QString::fromUtf8("草稿"),
        QString::fromUtf8("被标记"),
        QString::fromUtf8("还没有内容，快去记录真实瞬间吧。"),
        QString::fromUtf8("保存的草稿会出现在这里。"),
        QString::fromUtf8("好友标记你后，合集会出现在这里。"),
        QString::fromUtf8("关注"),
        QString::fromUtf8("粉丝"),
        QString::fromUtf8("点赞")
    };
}

namespace {
QWidget *createStatBadge(const QString &value,
                         const QString &label,
                         const QString &labelKey,
                         QWidget *parent,
                         std::vector<std::pair<QLabel *, QString>> *registry) {
    auto *wrapper = new QWidget(parent);
    auto *layout = new QVBoxLayout(wrapper);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *valueLabel = new QLabel(value, wrapper);
    valueLabel->setObjectName("statValue");
    auto *labelLabel = new QLabel(label, wrapper);
    labelLabel->setObjectName("statLabel");
    labelLabel->setProperty("statKey", labelKey);

    layout->addWidget(valueLabel, 0, Qt::AlignHCenter);
    layout->addWidget(labelLabel, 0, Qt::AlignHCenter);

    if (registry) {
        registry->push_back({labelLabel, labelKey});
    }

    return wrapper;
}
} // namespace

ProfilePage::ProfilePage(const std::vector<TheButtonInfo> &videos, QWidget *parent)
    : QWidget(parent),
      videos_(videos),
      gradientAngle_(0.0),
      scrollArea_(nullptr) {
    setObjectName("profilePageRoot");
    defaultStyleSheet_ = defaultProfileStyle();
    highContrastStyleSheet_ = highContrastProfileStyle();
    dayModeStyleSheet_ = dayModeProfileStyle();
    setStyleSheet(defaultStyleSheet_);

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto *scrollArea = new QScrollArea(this);
    scrollArea_ = scrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->viewport()->setAutoFillBackground(false);
    scrollArea->viewport()->setStyleSheet("background: transparent;");

    auto *contentWidget = new QWidget(scrollArea);
    auto *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(48, 48, 48, 160);
    contentLayout->setSpacing(32);

    auto *contentShell = new QFrame(contentWidget);
    contentShell->setObjectName("contentShell");
    auto *shellLayout = new QVBoxLayout(contentShell);
    shellLayout->setContentsMargins(40, 44, 40, 56);
    shellLayout->setSpacing(32);

    scrollArea->setWidget(contentWidget);
    rootLayout->addWidget(scrollArea);
    
    // 连接滚动条信号，使色环随滚动变化
    QObject::connect(scrollArea->verticalScrollBar(), &QAbstractSlider::valueChanged, 
                     [this, scrollArea](int value) {
        // 将滚动位置转换为角度（0-360度）
        const int maxScroll = scrollArea->verticalScrollBar()->maximum();
        if (maxScroll > 0) {
            gradientAngle_ = (static_cast<qreal>(value) / maxScroll) * 360.0;
        } else {
            gradientAngle_ = 0.0;
        }
        update(); // 触发重绘
    });

    auto *heroCard = new QFrame(contentShell);
    heroCard->setObjectName("heroCard");
    auto *heroLayout = new QVBoxLayout(heroCard);
    heroLayout->setContentsMargins(28, 28, 28, 28);
    heroLayout->setSpacing(20);

    identityRow_ = new QHBoxLayout();
    identityRow_->setSpacing(20);

    // 使用彩色环头像
    const QIcon *avatarIcon = videos.empty() ? nullptr : videos.front().icon;
    avatarWidget_ = new AvatarRingWidget(avatarIcon, heroCard);

    auto *identityCol = new QVBoxLayout();
    identityCol->setSpacing(8);

    displayNameLabel_ = new QLabel("Lina Mendes", heroCard);
    displayNameLabel_->setObjectName("displayName");
    usernameLabel_ = new QLabel("@linagoesreal", heroCard);
    usernameLabel_->setObjectName("username");

    auto *ctaRow = new QHBoxLayout();
    ctaRow->setSpacing(12);
    auto *followBtn = new QPushButton("Follow", heroCard);
    followBtn->setObjectName("primaryCta");
    followBtn->setCheckable(true);
    followBtn->setCursor(Qt::PointingHandCursor);
    // 添加图标（放大一些，更接近 Instagram 风格）
    followBtn->setIcon(QIcon(":/icons/icons/follow.svg"));
    followBtn->setIconSize(QSize(34, 34));
    followButton_ = followBtn;
    QObject::connect(followBtn, &QPushButton::toggled, this, [this](bool checked) {
        applyFollowState(checked);
        // 语音播报
        if (checked) {
            NarrationManager::instance().narrate(
                QString::fromUtf8("已关注"),
                "Following"
            );
        } else {
            NarrationManager::instance().narrate(
                QString::fromUtf8("取消关注"),
                "Unfollowed"
            );
        }
    });
    
    QObject::connect(shareButton_, &QPushButton::clicked, this, [this]() {
        NarrationManager::instance().narrate(
            QString::fromUtf8("分享主页"),
            "Share profile"
        );
    });

    shareButton_ = new QPushButton("Share profile", heroCard);
    shareButton_->setObjectName("secondaryCta");
    // 添加图标（放大一些）
    shareButton_->setIcon(QIcon(":/icons/icons/share_profile.svg"));
    shareButton_->setIconSize(QSize(30, 30));
    ctaRow->addWidget(followBtn);
    ctaRow->addWidget(shareButton_);
    ctaRow->addStretch();

    identityCol->addWidget(displayNameLabel_);
    identityCol->addWidget(usernameLabel_);
    identityCol->addLayout(ctaRow);

    identityRow_->addWidget(avatarWidget_);
    identityRow_->addLayout(identityCol);
    identityRow_->addStretch();

    statsRow_ = new QHBoxLayout();
    statsRow_->setSpacing(24);
    statsRow_->addWidget(createStatBadge("428", "Following", "stat.following", heroCard, &statLabelWidgets_));
    statsRow_->addWidget(createStatBadge("18.3K", "Followers", "stat.followers", heroCard, &statLabelWidgets_));
    statsRow_->addWidget(createStatBadge("1.2M", "Likes", "stat.likes", heroCard, &statLabelWidgets_));
    statsRow_->addStretch();

    bioLabel_ = new QLabel("Creator of rooftop BeReal drops • chasing golden hour on every continent • new lens drops every Sunday.", heroCard);
    bioLabel_->setObjectName("bioLabel");
    bioLabel_->setWordWrap(true);

    heroLayout->addLayout(identityRow_);
    heroLayout->addLayout(statsRow_);
    heroLayout->addWidget(bioLabel_);

    auto *filterFrame = new QFrame(contentShell);
    filterFrame->setObjectName("videoFilter");
    auto *filterLayout = new QHBoxLayout(filterFrame);
    filterLayout->setContentsMargins(20, 12, 20, 12);
    filterLayout->setSpacing(10);
    videosLabel_ = new QLabel("Replays", filterFrame);
    videosLabel_->setObjectName("sectionTitle");
    filterLayout->addWidget(videosLabel_);
    filterLayout->addStretch();

    struct FilterSpec {
        QString label;
        FilterMode mode;
        QString key;
    };
    const std::vector<FilterSpec> filterSpecs = {
        {"Grid", FilterMode::Grid, "grid"},
        {"Drafts", FilterMode::Drafts, "drafts"},
        {"Tagged", FilterMode::Tagged, "tagged"}
    };

    auto *filterGroup = new QButtonGroup(filterFrame);
    filterGroup->setExclusive(true);
    filterButtons_.reserve(filterSpecs.size());
    for (const auto &spec : filterSpecs) {
        auto *tab = new QPushButton(spec.label, filterFrame);
        tab->setObjectName("filterTab");
        tab->setCheckable(true);
        tab->setCursor(Qt::PointingHandCursor);
        tab->setProperty("filterKey", spec.key);
        filterGroup->addButton(tab, static_cast<int>(spec.mode));
        filterLayout->addWidget(tab);
        filterButtons_.push_back(tab);

        QObject::connect(tab, &QPushButton::toggled, this, [this, index = static_cast<int>(spec.mode), &spec](bool checked) {
            if (checked) {
                setActiveFilter(index);
                // 语音播报
                QString zhText, enText;
                if (index == FilterMode::Grid) {
                    zhText = QString::fromUtf8("网格");
                    enText = "Grid";
                } else if (index == FilterMode::Drafts) {
                    zhText = QString::fromUtf8("草稿");
                    enText = "Drafts";
                } else if (index == FilterMode::Tagged) {
                    zhText = QString::fromUtf8("被标记");
                    enText = "Tagged";
                }
                NarrationManager::instance().narrate(zhText, enText);
            }
        });
    }

    gridFrame_ = new QFrame(contentShell);
    gridFrame_->setObjectName("videoGrid");
    gridLayout_ = new QGridLayout(gridFrame_);
    gridLayout_->setContentsMargins(18, 18, 18, 18);
    gridLayout_->setSpacing(14);
    emptyStateLabel_ = new QLabel(gridFrame_);
    emptyStateLabel_->setStyleSheet("color: #7b8fad; font-size: 16px;");
    emptyStateLabel_->setWordWrap(true);
    emptyStateLabel_->hide();

    // Profile 网格只展示最新的 8 个视频（例如 n–u），而不是最早的几个
    const int total = std::min<int>(videos_.size(), 8);
    gridOrder_.reserve(total);
    draftOrder_.reserve((total + 1) / 2);
    taggedOrder_.reserve(total / 2);

    const int startIndex = static_cast<int>(videos_.size()) - total; // 取最后 total 个
    for (int offset = 0; offset < total; ++offset) {
        const int idx = startIndex + offset;
        gridOrder_.push_back(idx);
        if (offset % 2 == 0) {
            draftOrder_.push_back(idx);
        } else {
            taggedOrder_.push_back(idx);
        }
    }
    if (draftOrder_.empty() && !gridOrder_.empty()) {
        draftOrder_.push_back(gridOrder_.front());
    }
    if (taggedOrder_.empty() && gridOrder_.size() > 1) {
        taggedOrder_.push_back(gridOrder_.back());
    }

    if (!filterButtons_.empty()) {
        filterButtons_.front()->setChecked(true);
    } else {
        setActiveFilter(static_cast<int>(FilterMode::Grid));
    }

    shellLayout->addWidget(heroCard);
    shellLayout->addWidget(filterFrame);
    shellLayout->addWidget(gridFrame_);
    shellLayout->addStretch(1);

    contentLayout->addWidget(contentShell);
    contentLayout->addStretch(1);

    rebuildStrings();
    updateResponsiveLayout();
}

void ProfilePage::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    updateResponsiveLayout();
}

void ProfilePage::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制彩色边框（和Homepage一样）
    // 外层矩形略微内缩，避免被裁剪
    QRectF outerRect = rect().adjusted(3.0, 3.0, -3.0, -3.0);
    
    // 使用和头像类似的多色渐变做一圈「色环边框」
    // 渐变角度随滚动位置变化
    QConicalGradient grad(outerRect.center(), gradientAngle_);
    grad.setColorAt(0.00, QColor("#FF4F70"));
    grad.setColorAt(0.18, QColor("#FF8AA0"));
    grad.setColorAt(0.32, QColor("#6CADFF"));
    grad.setColorAt(0.46, QColor("#3A7DFF"));
    grad.setColorAt(0.60, QColor("#BFBFBF"));
    grad.setColorAt(0.74, QColor("#6CADFF"));
    grad.setColorAt(0.88, QColor("#FF8AA0"));
    grad.setColorAt(1.00, QColor("#FF4F70"));
    
    QPen pen(QBrush(grad), 5.0);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    
    const qreal radius = 10.0;
    painter.drawRoundedRect(outerRect, radius, radius);
}

void ProfilePage::updateResponsiveLayout() {
    if (!identityRow_ || !statsRow_ || !gridLayout_) {
        return;
    }

    const int w = width();
    const bool compactIdentity = w < 900;
    identityRow_->setDirection(compactIdentity ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight);
    statsRow_->setDirection(w < 720 ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight);

    int columns = 3;
    if (w < 580) {
        columns = 1;
    } else if (w < 980) {
        columns = 2;
    } else if (w >= 1400) {
        columns = 4;
    }

    if (videoTiles_.empty()) {
        return;
    }

    const int tileWidth = columns == 1 ? std::min(w - 120, 420) : columns == 4 ? 190 : 230;
    const int tileHeight = columns == 1 ? 360 : 300;

    for (int i = 0; i < static_cast<int>(videoTiles_.size()); ++i) {
        auto *tile = videoTiles_.at(i);
        tile->setFixedSize(tileWidth, tileHeight);
        tile->setIconSize(tile->size());
        gridLayout_->addWidget(tile, i / columns, i % columns);
    }
}

void ProfilePage::rebuildGrid() {
    if (!gridLayout_) {
        return;
    }

    for (auto *tile : videoTiles_) {
        gridLayout_->removeWidget(tile);
        tile->deleteLater();
    }
    videoTiles_.clear();

    if (emptyStateLabel_) {
        gridLayout_->removeWidget(emptyStateLabel_);
        emptyStateLabel_->hide();
    }

    const auto &order = orderFor(static_cast<int>(activeFilter_));
    if (order.empty()) {
        if (emptyStateLabel_) {
            emptyStateLabel_->setText(emptyStateFor(static_cast<int>(activeFilter_)));
            gridLayout_->addWidget(emptyStateLabel_, 0, 0, 1, 1);
            gridLayout_->setAlignment(emptyStateLabel_, Qt::AlignCenter);
            emptyStateLabel_->show();
        }
        return;
    }

    videoTiles_.reserve(order.size());
    for (int index : order) {
        if (index < 0 || index >= static_cast<int>(videos_.size())) {
            continue;
        }
        auto *tile = new QPushButton(gridFrame_);
        tile->setObjectName("videoTile");
        tile->setCursor(Qt::PointingHandCursor);
        tile->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        tile->setMinimumSize(180, 260);
        tile->setIconSize(QSize(180, 260));
        const auto &videoInfo = videos_.at(index);
        if (videoInfo.icon) {
            tile->setIcon(*videoInfo.icon);
        }
        QObject::connect(tile, &QPushButton::clicked, this, [this, index]() {
            emit playVideoRequested(index);
            // 语音播报
            NarrationManager::instance().narrate(
                QString::fromUtf8("播放视频 %1").arg(index + 1),
                QString("Play video %1").arg(index + 1)
            );
        });
        videoTiles_.push_back(tile);
    }

    updateResponsiveLayout();
}

void ProfilePage::setActiveFilter(int modeIndex) {
    const auto mode = static_cast<FilterMode>(modeIndex);
    if (activeFilter_ == mode) {
        return;
    }
    activeFilter_ = mode;
    rebuildGrid();
}

void ProfilePage::applyFollowState(bool following) {
    if (!followButton_) {
        return;
    }
    followButton_->setText(following ? followingLabel_ : followLabel_);
    if (!following && followButton_->isChecked()) {
        followButton_->blockSignals(true);
        followButton_->setChecked(false);
        followButton_->blockSignals(false);
    }
}

const std::vector<int> &ProfilePage::orderFor(int modeIndex) const {
    switch (static_cast<FilterMode>(modeIndex)) {
        case FilterMode::Grid:
            return gridOrder_;
        case FilterMode::Drafts:
            return draftOrder_;
        case FilterMode::Tagged:
            return taggedOrder_;
    }
    return gridOrder_;
}

QString ProfilePage::emptyStateFor(int modeIndex) const {
    switch (static_cast<FilterMode>(modeIndex)) {
        case FilterMode::Grid:
            return emptyGridLabel_;
        case FilterMode::Drafts:
            return emptyDraftsLabel_;
        case FilterMode::Tagged:
            return emptyTaggedLabel_;
    }
    return {};
}

void ProfilePage::setLanguage(AppLanguage language) {
    if (language_ == language) {
        rebuildStrings();
        return;
    }
    language_ = language;
    rebuildStrings();
    // 语音播报语言切换
    NarrationManager::instance().setLanguage(language);
}

void ProfilePage::rebuildStrings() {
    const ProfileCopy copy = localizedProfileCopy(language_);
    displayNameText_ = copy.displayName;
    usernameText_ = copy.username;
    bioText_ = copy.bio;
    followLabel_ = copy.follow;
    followingLabel_ = copy.following;
    shareLabel_ = copy.share;
    replaysLabel_ = copy.replays;
    gridLabel_ = copy.filterGrid;
    draftsLabel_ = copy.filterDrafts;
    taggedLabel_ = copy.filterTagged;
    emptyGridLabel_ = copy.emptyGrid;
    emptyDraftsLabel_ = copy.emptyDrafts;
    emptyTaggedLabel_ = copy.emptyTagged;

    if (displayNameLabel_) {
        displayNameLabel_->setText(displayNameText_);
    }
    if (usernameLabel_) {
        usernameLabel_->setText(usernameText_);
    }
    if (bioLabel_) {
        bioLabel_->setText(bioText_);
    }
    if (shareButton_) {
        shareButton_->setText(shareLabel_);
    }
    if (videosLabel_) {
        videosLabel_->setText(replaysLabel_);
    }
    for (auto *button : filterButtons_) {
        const QString key = button->property("filterKey").toString();
        if (key == "grid") {
            button->setText(gridLabel_);
        } else if (key == "drafts") {
            button->setText(draftsLabel_);
        } else if (key == "tagged") {
            button->setText(taggedLabel_);
        }
    }

    for (auto &entry : statLabelWidgets_) {
        if (!entry.first) {
            continue;
        }
        if (entry.second == "stat.following") {
            entry.first->setText(copy.statFollowing);
        } else if (entry.second == "stat.followers") {
            entry.first->setText(copy.statFollowers);
        } else if (entry.second == "stat.likes") {
            entry.first->setText(copy.statLikes);
        }
    }

    applyFollowState(followButton_ && followButton_->isChecked());
    rebuildGrid();
}

void ProfilePage::setHighContrastMode(bool enabled) {
    if (highContrastMode_ == enabled) {
        return;
    }
    highContrastMode_ = enabled;
    updateStyleSheet();
}

void ProfilePage::setDayMode(bool dayMode) {
    if (dayMode_ == dayMode) {
        return;
    }
    dayMode_ = dayMode;
    updateStyleSheet();
}

void ProfilePage::updateStyleSheet() {
    if (highContrastMode_) {
        setStyleSheet(highContrastStyleSheet_);
    } else if (dayMode_) {
        setStyleSheet(dayModeStyleSheet_);
    } else {
        setStyleSheet(defaultStyleSheet_);
    }

    // 根据模式切换高对比图标：夜间/高对比用 *1.svg，日间用默认
    const bool useAltIcons = highContrastMode_ || !dayMode_;
    if (followButton_) {
        followButton_->setIcon(QIcon(useAltIcons ? QStringLiteral(":/icons/icons/follow1.svg")
                                                 : QStringLiteral(":/icons/icons/follow.svg")));
    }
    if (shareButton_) {
        shareButton_->setIcon(QIcon(useAltIcons ? QStringLiteral(":/icons/icons/share_profile1.svg")
                                                : QStringLiteral(":/icons/icons/share_profile.svg")));
    }
}

void ProfilePage::setUserInfo(const QString &username, const QString &displayName, const QString &bio, const QString &avatarPath) {
    currentUsername_ = username;
    
    // 更新头像
    if (avatarWidget_) {
        QString finalAvatarPath = avatarPath;
        if (finalAvatarPath.isEmpty() && !username.isEmpty()) {
            finalAvatarPath = getAvatarPathForUser(username);
        }
        qDebug() << "[ProfilePage::setUserInfo] username:" << username << "avatarPath:" << finalAvatarPath;
        if (!finalAvatarPath.isEmpty()) {
            if (QFile::exists(finalAvatarPath)) {
                avatarWidget_->setAvatarFromPath(finalAvatarPath);
                qDebug() << "[ProfilePage::setUserInfo] 成功设置头像路径:" << finalAvatarPath;
            } else {
                qDebug() << "[ProfilePage::setUserInfo] 头像文件不存在:" << finalAvatarPath;
            }
        } else {
            qDebug() << "[ProfilePage::setUserInfo] 未找到头像路径";
        }
    }
    
    if (!username.isEmpty()) {
        usernameText_ = "@" + username;
        if (usernameLabel_) {
            usernameLabel_->setText(usernameText_);
        }
    }
    
    if (!displayName.isEmpty()) {
        displayNameText_ = displayName;
        if (displayNameLabel_) {
            displayNameLabel_->setText(displayNameText_);
        }
    } else if (!username.isEmpty()) {
        // 如果没有提供displayName，使用username的首字母大写
        QString capitalized = username;
        if (!capitalized.isEmpty()) {
            capitalized[0] = capitalized[0].toUpper();
        }
        displayNameText_ = capitalized;
        if (displayNameLabel_) {
            displayNameLabel_->setText(displayNameText_);
        }
    }
    
    if (!bio.isEmpty()) {
        bioText_ = bio;
        if (bioLabel_) {
            bioLabel_->setText(bioText_);
        }
    } else if (!username.isEmpty()) {
        // 生成默认bio
        bioText_ = QString("Welcome to %1's profile!").arg(username);
        if (bioLabel_) {
            bioLabel_->setText(bioText_);
        }
    }
    
    // 为不同用户生成不同的统计数据
    if (!username.isEmpty() && !statLabelWidgets_.empty()) {
        // 根据用户名生成不同的统计数据（使用哈希确保一致性）
        uint hash = qHash(username);
        int following = 200 + (hash % 500);  // 200-700
        int followers = 1000 + ((hash * 3) % 20000);  // 1K-21K
        int likes = 50000 + ((hash * 7) % 950000);  // 50K-1M
        
        // 格式化数字
        QString followingStr = QString::number(following);
        QString followersStr;
        if (followers >= 1000) {
            followersStr = QString::number(followers / 1000.0, 'f', 1) + "K";
        } else {
            followersStr = QString::number(followers);
        }
        QString likesStr;
        if (likes >= 1000000) {
            likesStr = QString::number(likes / 1000000.0, 'f', 1) + "M";
        } else if (likes >= 1000) {
            likesStr = QString::number(likes / 1000.0, 'f', 1) + "K";
        } else {
            likesStr = QString::number(likes);
        }
        
        // 更新统计数据标签
        for (auto &pair : statLabelWidgets_) {
            QString key = pair.second;
            if (key == "stat.following" && pair.first) {
                pair.first->setText(followingStr);
            } else if (key == "stat.followers" && pair.first) {
                pair.first->setText(followersStr);
            } else if (key == "stat.likes" && pair.first) {
                pair.first->setText(likesStr);
            }
        }
    }
}


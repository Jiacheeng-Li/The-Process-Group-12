#ifndef PROFILE_H
#define PROFILE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>

// 圆形头像组件
class CircleAvatar : public QWidget {
    Q_OBJECT
public:
    explicit CircleAvatar(QWidget *parent = nullptr, int size = 100);
    void setAvatar(const QString &path);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_size;
    QString m_avatarPath;
};

// 统计数字卡片组件
class StatCard : public QWidget {
    Q_OBJECT
public:
    explicit StatCard(const QString &title, int count, QWidget *parent = nullptr);

private:
    QLabel *m_countLabel;
    QLabel *m_titleLabel;
};

// 视频卡片组件
class VideoCard : public QWidget {
    Q_OBJECT
public:
    explicit VideoCard(const QString &coverPath = "", QWidget *parent = nullptr);

private:
    void setDefaultCover(); // 设置默认视频封面

private:
    QLabel *m_coverLabel;
};

// 主窗口（TikTok个人主页）
class TikTokProfile : public QWidget {
    Q_OBJECT
public:
    explicit TikTokProfile(QWidget *parent = nullptr);

private:
    void initUI(); // 初始化界面

private:
    CircleAvatar *m_avatar; // 头像组件
};

#endif // TIKTOKPROFILE_H

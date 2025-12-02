#ifndef FRIENDITEM_H
#define FRIENDITEM_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QDateTime>
#include <QEvent>
#include <QResizeEvent>
#include "../shared/language_manager.h"

struct FriendPostCopy {
    QString tagZh = "#日常瞬间";
    QString tagEn = "#DailyMoments";
    QString captionZh = "分享一个新视频，欢迎来聊！";
    QString captionEn = "Just dropped a new clip, come say hi!";
};

class FriendItem : public QWidget
{
    Q_OBJECT
public:
    explicit FriendItem(const QString &avatarPath,
                        const QString &username,
                        const QString &videoThumb,
                        const QDateTime &time = QDateTime::currentDateTime(),
                        int videoIndex = -1,  // 视频索引，-1表示无视频
                        const FriendPostCopy &copy = FriendPostCopy(),
                        QWidget *parent = nullptr);

    // 发布页注入一条“我自己”
    static FriendItem* fromPublish(const QString &videoThumb,
                                   const FriendPostCopy &copy = FriendPostCopy());

    void setDayMode(bool dayMode);
    void setHighContrastMode(bool enabled);

signals:
    void commentRequested(FriendItem *self);
    void avatarClicked(const QString &username);
    void thumbnailClicked(int videoIndex);  // 缩略图被点击时发出视频索引

public slots:
    void addComment(const QString &text, const QString &commenter = "You");  // 添加评论，支持指定评论者
    void setThumbnail(const QPixmap &pixmap);  // 设置缩略图
    void updateThumbnailSize();  // 公开方法，允许外部触发更新

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void onLike();
    void onShare();
    void onRepost();
    void onComment();
    void applyLanguage(AppLanguage lang);

private:
    QLabel *avatar;
    QLabel *usernameLbl;
    QLabel *timeLbl;
    QLabel *tagLbl;
    QLabel *contentLbl;
    QLabel *thumbLbl;

    QPushButton *likeBtn;
    QPushButton *commentBtn;
    QPushButton *shareBtn;
    QPushButton *repostBtn;

    QWidget *commentArea;  // 评论区容器
    QLineEdit *commentInput;  // 评论输入框

    int likeCount = 0;
    int commentCount = 0;
    int shareCount = 0;
    int repostCount = 0;
    bool liked = false;
    bool shared = false;
    bool reposted = false;

    QString thumbPath;
    QString username_;  // 存储用户名
    int videoIndex_;  // 存储视频索引
    FriendPostCopy copy_;
    
    // 性能优化：缓存上次的尺寸，避免重复缩放
    int lastThumbWidth_ = -1;
    int lastThumbHeight_ = -1;
    QPixmap originalThumbPixmap_;  // 缓存原始缩略图

    bool dayMode_ = false;
    bool highContrast_ = false;

    void updateCountDisplay();
    void applyThemeStyles();
    void restyleCommentLabels();
    void restyleCommentLabel(QLabel *label);
};

#endif // FRIENDITEM_H

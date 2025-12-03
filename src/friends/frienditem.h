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
                        int videoIndex = -1,  // Video index, -1 means no video
                        const FriendPostCopy &copy = FriendPostCopy(),
                        QWidget *parent = nullptr);

    // Inject a "myself" item from publish page
    static FriendItem* fromPublish(const QString &videoThumb,
                                   const FriendPostCopy &copy = FriendPostCopy());

    void setDayMode(bool dayMode);
    void setHighContrastMode(bool enabled);

signals:
    void commentRequested(FriendItem *self);
    void avatarClicked(const QString &username);
    void thumbnailClicked(int videoIndex);  // Emit video index when thumbnail is clicked

public slots:
    void addComment(const QString &text, const QString &commenter = "You");  // Add comment, supports specifying commenter
    void setThumbnail(const QPixmap &pixmap);  // Set thumbnail
    void updateThumbnailSize();  // Public method, allows external trigger update

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

    QWidget *commentArea;  // Comment area container
    QLineEdit *commentInput;  // Comment input box

    int likeCount = 0;
    int commentCount = 0;
    int shareCount = 0;
    int repostCount = 0;
    bool liked = false;
    bool shared = false;
    bool reposted = false;

    QString thumbPath;
    QString username_;  // Store username
    int videoIndex_;  // Store video index
    FriendPostCopy copy_;
    
    // Performance optimization: cache last size to avoid repeated scaling
    int lastThumbWidth_ = -1;
    int lastThumbHeight_ = -1;
    QPixmap originalThumbPixmap_;  // Cache original thumbnail

    bool dayMode_ = false;
    bool highContrast_ = false;

    void updateCountDisplay();
    void applyThemeStyles();
    void restyleCommentLabels();
    void restyleCommentLabel(QLabel *label);
};

#endif // FRIENDITEM_H

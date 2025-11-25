#ifndef FRIENDITEM_H
#define FRIENDITEM_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QDateTime>
#include <QEvent>

class FriendItem : public QWidget
{
    Q_OBJECT
public:
    explicit FriendItem(const QString &avatarPath,
                        const QString &username,
                        const QString &videoThumb,
                        const QDateTime &time = QDateTime::currentDateTime(),
                        QWidget *parent = nullptr);

    // 发布页注入一条“我自己”
    static FriendItem* fromPublish(const QString &videoThumb);

signals:
    void commentRequested(FriendItem *self);
    void avatarClicked(const QString &username);

public slots:
    void addComment(const QString &text);
    void setThumbnail(const QPixmap &pixmap);  // 设置缩略图

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onLike();
    void onShare();
    void onComment();

private:
    QLabel *avatar;
    QLabel *usernameLbl;
    QLabel *timeLbl;
    QLabel *thumbLbl;

    QPushButton *likeBtn;
    QPushButton *commentBtn;
    QPushButton *shareBtn;

    int likeCount = 0;
    int commentCount = 0;
    int shareCount = 0;
    bool liked = false;
    bool shared = false;

    QString thumbPath;

    void updateCountDisplay();
};

#endif // FRIENDITEM_H

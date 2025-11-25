#ifndef FRIENDSPAGE_H
#define FRIENDSPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QString>
#include <vector>
#include "frienditem.h"
#include "../player/the_button.h"  // 需要完整定义才能使用 std::vector<TheButtonInfo>

class FriendsPage : public QWidget
{
    Q_OBJECT
public:
    explicit FriendsPage(QWidget *parent = nullptr);
    // 使用视频列表初始化
    explicit FriendsPage(const std::vector<TheButtonInfo> &videos, QWidget *parent = nullptr);
    void addNewPost(const QString &videoThumb);
    void setVideos(const std::vector<TheButtonInfo> &videos);

signals:
    void goToProfile(const QString &username);

private slots:
    void onCommentRequested(FriendItem *item);

private:
    void initializeWithVideos(const std::vector<TheButtonInfo> &videos);
    
    QWidget *listWidget;
    QVBoxLayout *listLayout;
    QScrollArea *scrollArea;
    std::vector<TheButtonInfo> videoList;
};

#endif // FRIENDSPAGE_H

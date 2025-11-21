#ifndef FRIENDSPAGE_H
#define FRIENDSPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include "frienditem.h"

class FriendsPage : public QWidget
{
    Q_OBJECT
public:
    explicit FriendsPage(QWidget *parent = nullptr);
    void addNewPost(const QString &videoThumb);

signals:
    void goToProfile(const QString &username);

private slots:
    void onCommentRequested(FriendItem *item);

private:
    QWidget *listWidget;
    QVBoxLayout *listLayout;
    QScrollArea *scrollArea;
};

#endif // FRIENDSPAGE_H

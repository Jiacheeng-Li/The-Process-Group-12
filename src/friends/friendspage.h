#ifndef FRIENDSPAGE_H
#define FRIENDSPAGE_H

#include <QWidget>
#include <QListView>

class FriendsPage : public QWidget
{
    Q_OBJECT
public:
    explicit FriendsPage(QWidget *parent = nullptr);

private:
    QListView *feedListView;
    
    // TODO: Add timeline display
    // TODO: Add friend video list
};

#endif // FRIENDSPAGE_H

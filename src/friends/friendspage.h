#ifndef FRIENDSPAGE_H
#define FRIENDSPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QString>
#include <QResizeEvent>
#include <vector>
#include "frienditem.h"
#include "../player/the_button.h"  // Needs full definition for std::vector<TheButtonInfo>

class FriendsPage : public QWidget
{
    Q_OBJECT
public:
    explicit FriendsPage(QWidget *parent = nullptr);
    // Initialize with video list
    explicit FriendsPage(const std::vector<TheButtonInfo> &videos, QWidget *parent = nullptr);
    void addNewPost(const QString &videoThumb);
    void setVideos(const std::vector<TheButtonInfo> &videos);
    void setDayMode(bool dayMode);
    void setHighContrastMode(bool enabled);

signals:
    void goToProfile(const QString &username);
    void playVideoRequested(int index);  // Notifies that a feed card requested playback

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onCommentRequested(FriendItem *item);

private:
    void initializeWithVideos(const std::vector<TheButtonInfo> &videos);
    void updateResponsiveLayout();
    void applyThemeStyles();
    void propagateThemeToItems();
    
    QWidget *listWidget;
    QVBoxLayout *listLayout;
    QScrollArea *scrollArea;
    std::vector<TheButtonInfo> videoList;
    bool dayMode_ = false;
    bool highContrastMode_ = false;
};

#endif // FRIENDSPAGE_H

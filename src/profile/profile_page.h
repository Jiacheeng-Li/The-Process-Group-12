#ifndef PROFILE_PAGE_H
#define PROFILE_PAGE_H

#include <QWidget>
#include <vector>
#include <utility>
#include <QString>

#include "the_button.h"
#include "app_settings.h"

class QHBoxLayout;
class QGridLayout;
class QPushButton;
class QLabel;
class QResizeEvent;
class QScrollArea;
class AvatarRingWidget;

class ProfilePage : public QWidget {
    Q_OBJECT

public:
    explicit ProfilePage(const std::vector<TheButtonInfo> &videos, QWidget *parent = nullptr);
    void setLanguage(AppLanguage language);
    void setHighContrastMode(bool enabled);
    void setDayMode(bool dayMode);
    void setUserInfo(const QString &username, const QString &displayName = "", const QString &bio = "", const QString &avatarPath = "");

signals:
    void playVideoRequested(int index);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void updateResponsiveLayout();
    void rebuildGrid();
    void setActiveFilter(int modeIndex);
    void applyFollowState(bool following);
    const std::vector<int> &orderFor(int modeIndex) const;
    QString emptyStateFor(int modeIndex) const;
    void rebuildStrings();
    void updateStyleSheet();

    enum FilterMode { Grid = 0, Drafts = 1, Tagged = 2 };

    const std::vector<TheButtonInfo> videos_;
    FilterMode activeFilter_ = Grid;

    QHBoxLayout *identityRow_ = nullptr;
    QHBoxLayout *statsRow_ = nullptr;
    QGridLayout *gridLayout_ = nullptr;
    QWidget *gridFrame_ = nullptr;
    QLabel *emptyStateLabel_ = nullptr;
    QPushButton *followButton_ = nullptr;
    std::vector<QPushButton *> filterButtons_;
    std::vector<QPushButton *> videoTiles_;
    std::vector<int> gridOrder_;
    std::vector<int> draftOrder_;
    std::vector<int> taggedOrder_;
    QLabel *displayNameLabel_ = nullptr;
    QLabel *usernameLabel_ = nullptr;
    QLabel *bioLabel_ = nullptr;
    QLabel *videosLabel_ = nullptr;
    QPushButton *shareButton_ = nullptr;
    AppLanguage language_ = AppLanguage::Chinese;
    bool highContrastMode_ = false;
    bool dayMode_ = false;
    QString defaultStyleSheet_;
    QString highContrastStyleSheet_;
    QString dayModeStyleSheet_;
    QString followLabel_;
    QString followingLabel_;
    QString shareLabel_;
    QString replaysLabel_;
    QString gridLabel_;
    QString draftsLabel_;
    QString taggedLabel_;
    QString emptyGridLabel_;
    QString emptyDraftsLabel_;
    QString emptyTaggedLabel_;
    QString bioText_;
    QString usernameText_;
    QString displayNameText_;
    std::vector<std::pair<QLabel *, QString>> statLabelWidgets_;
    qreal gradientAngle_; // Gradient angle, changes with scrolling
    QScrollArea *scrollArea_; // Save scroll area pointer for connecting signals
    AvatarRingWidget *avatarWidget_ = nullptr; // Save avatar widget pointer for updating avatar
    QString currentUsername_; // Currently displayed username
};

#endif // PROFILE_PAGE_H


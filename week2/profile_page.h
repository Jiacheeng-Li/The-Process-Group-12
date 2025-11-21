#ifndef PROFILE_PAGE_H
#define PROFILE_PAGE_H

#include <QWidget>
#include <vector>

#include "the_button.h"

class QHBoxLayout;
class QGridLayout;
class QPushButton;
class QLabel;
class QResizeEvent;

class ProfilePage : public QWidget {
    Q_OBJECT

public:
    explicit ProfilePage(const std::vector<TheButtonInfo> &videos, QWidget *parent = nullptr);

signals:
    void playVideoRequested(int index);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateResponsiveLayout();
    void rebuildGrid();
    void setActiveFilter(int modeIndex);
    void applyFollowState(bool following);
    const std::vector<int> &orderFor(int modeIndex) const;
    QString emptyStateFor(int modeIndex) const;

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
};

#endif // PROFILE_PAGE_H


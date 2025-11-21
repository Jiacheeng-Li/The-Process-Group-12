#include "profile_page.h"

#include <QButtonGroup>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QSizePolicy>
#include <QResizeEvent>
#include <QScrollArea>
#include <QVBoxLayout>
#include <algorithm>

namespace {
QPixmap roundedFromIcon(const QIcon *icon, const QSize &size, int radius) {
    QPixmap base(size);
    base.fill(Qt::transparent);

    if (icon) {
        QPixmap pix = icon->pixmap(size);
        pix = pix.scaled(size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QPainter painter(&base);
        painter.setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        path.addRoundedRect(QRectF(0, 0, size.width(), size.height()), radius, radius);
        painter.setClipPath(path);
        painter.drawPixmap(0, 0, pix);
        painter.end();
    } else {
        base.fill(QColor("#2d2d2d"));
    }

    return base;
}

QWidget *createStatBadge(const QString &value, const QString &label, QWidget *parent) {
    auto *wrapper = new QWidget(parent);
    auto *layout = new QVBoxLayout(wrapper);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *valueLabel = new QLabel(value, wrapper);
    valueLabel->setObjectName("statValue");
    auto *labelLabel = new QLabel(label, wrapper);
    labelLabel->setObjectName("statLabel");

    layout->addWidget(valueLabel, 0, Qt::AlignHCenter);
    layout->addWidget(labelLabel, 0, Qt::AlignHCenter);
    return wrapper;
}
} // namespace

ProfilePage::ProfilePage(const std::vector<TheButtonInfo> &videos, QWidget *parent)
    : QWidget(parent),
      videos_(videos) {
    setObjectName("profilePageRoot");
    setStyleSheet(
        "QWidget#profilePageRoot {"
        "  background: qradialgradient(cx:0.25, cy:0.2, radius:1.25,"
        "    stop:0 #091230, stop:0.5 #030918, stop:1 #00040a);"
        "}"
        "QScrollArea { background: transparent; border: none; }"
        "QScrollArea > QWidget > QWidget { background: transparent; }"
        "QFrame#contentShell {"
        "  background: rgba(4,12,26,0.9);"
        "  border-radius: 48px;"
        "  border: 1px solid rgba(63,134,255,0.35);"
        "  outline: 1px solid rgba(10,18,40,0.6);"
        "}"
        "QFrame#heroCard {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #0b1526, stop:1 #040714);"
        "  border-radius: 32px;"
        "  border: 1px solid rgba(83,164,255,0.35);"
        "}"
        "QLabel#displayName { font-size: 30px; font-weight: 700; color: white; }"
        "QLabel#username { font-size: 15px; color: #6f84a8; }"
        "QLabel#bioLabel { color: #cfd9ec; line-height: 1.6; }"
        "QLabel#statValue { font-size: 20px; font-weight: 700; color: white; }"
        "QLabel#statLabel { font-size: 12px; text-transform: uppercase; color: #6a819f; letter-spacing: 0.2em; }"
        "QPushButton#primaryCta {"
        "  background-color: #2f8dff;"
        "  border-radius: 22px;"
        "  padding: 12px 26px;"
        "  border: none;"
        "  color: white;"
        "  font-weight: 700;"
        "}"
        "QPushButton#primaryCta:hover { background-color: #4ca2ff; }"
        "QPushButton#primaryCta:checked {"
        "  background-color: rgba(22,37,66,0.85);"
        "  border: 1px solid rgba(76,162,255,0.8);"
        "  color: #9db6ff;"
        "}"
        "QPushButton#secondaryCta {"
        "  background-color: transparent;"
        "  color: white;"
        "  border: 1px solid rgba(76,162,255,0.5);"
        "  border-radius: 22px;"
        "  padding: 11px 24px;"
        "  font-weight: 600;"
        "}"
        "QFrame#videoFilter {"
        "  background: rgba(6,14,28,0.9);"
        "  border-radius: 20px;"
        "  border: 1px solid rgba(63,134,255,0.25);"
        "}"
        "QFrame#videoGrid {"
        "  background: rgba(5,10,20,0.92);"
        "  border-radius: 30px;"
        "  border: 1px solid rgba(63,134,255,0.25);"
        "  padding: 6px;"
        "}"
        "QPushButton#filterTab {"
        "  background-color: transparent;"
        "  color: #6f84a8;"
        "  border: none;"
        "  font-weight: 600;"
        "  padding: 8px 16px;"
        "  border-radius: 14px;"
        "}"
        "QPushButton#filterTab:hover { color: #d3ddff; }"
        "QPushButton#filterTab:checked {"
        "  color: white;"
        "  background-color: rgba(47,141,255,0.22);"
        "}"
        "QPushButton#videoTile {"
        "  background-color: #0b1326;"
        "  border: 1px solid rgba(87,168,255,0.18);"
        "  border-radius: 20px;"
        "  min-height: 260px;"
        "}"
        "QPushButton#videoTile:hover {"
        "  border: 1px solid rgba(76,162,255,0.5);"
        "}"
        "QLabel#sectionTitle { font-size: 20px; font-weight: 700; color: white; }");

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->viewport()->setAutoFillBackground(false);
    scrollArea->viewport()->setStyleSheet("background: transparent;");

    auto *contentWidget = new QWidget(scrollArea);
    auto *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(48, 48, 48, 160);
    contentLayout->setSpacing(32);

    auto *contentShell = new QFrame(contentWidget);
    contentShell->setObjectName("contentShell");
    auto *shellLayout = new QVBoxLayout(contentShell);
    shellLayout->setContentsMargins(40, 44, 40, 56);
    shellLayout->setSpacing(32);

    scrollArea->setWidget(contentWidget);
    rootLayout->addWidget(scrollArea);

    auto *heroCard = new QFrame(contentShell);
    heroCard->setObjectName("heroCard");
    auto *heroLayout = new QVBoxLayout(heroCard);
    heroLayout->setContentsMargins(28, 28, 28, 28);
    heroLayout->setSpacing(20);

    identityRow_ = new QHBoxLayout();
    identityRow_->setSpacing(20);

    auto *avatar = new QLabel(heroCard);
    avatar->setFixedSize(120, 120);
    avatar->setScaledContents(true);
    avatar->setPixmap(roundedFromIcon(videos.empty() ? nullptr : videos.front().icon, avatar->size(), 60));

    auto *identityCol = new QVBoxLayout();
    identityCol->setSpacing(8);

    auto *displayName = new QLabel("Lina Mendes", heroCard);
    displayName->setObjectName("displayName");
    auto *username = new QLabel("@linagoesreal", heroCard);
    username->setObjectName("username");

    auto *ctaRow = new QHBoxLayout();
    ctaRow->setSpacing(12);
    auto *followBtn = new QPushButton("Follow", heroCard);
    followBtn->setObjectName("primaryCta");
    followBtn->setCheckable(true);
    followBtn->setCursor(Qt::PointingHandCursor);
    followButton_ = followBtn;
    QObject::connect(followBtn, &QPushButton::toggled, this, [this](bool checked) {
        applyFollowState(checked);
    });

    auto *shareBtn = new QPushButton("Share profile", heroCard);
    shareBtn->setObjectName("secondaryCta");
    ctaRow->addWidget(followBtn);
    ctaRow->addWidget(shareBtn);
    ctaRow->addStretch();

    identityCol->addWidget(displayName);
    identityCol->addWidget(username);
    identityCol->addLayout(ctaRow);

    identityRow_->addWidget(avatar);
    identityRow_->addLayout(identityCol);
    identityRow_->addStretch();

    statsRow_ = new QHBoxLayout();
    statsRow_->setSpacing(24);
    statsRow_->addWidget(createStatBadge("428", "Following", heroCard));
    statsRow_->addWidget(createStatBadge("18.3K", "Followers", heroCard));
    statsRow_->addWidget(createStatBadge("1.2M", "Likes", heroCard));
    statsRow_->addStretch();

    auto *bio = new QLabel("Creator of rooftop BeReal drops • chasing golden hour on every continent • new lens drops every Sunday.", heroCard);
    bio->setObjectName("bioLabel");
    bio->setWordWrap(true);

    heroLayout->addLayout(identityRow_);
    heroLayout->addLayout(statsRow_);
    heroLayout->addWidget(bio);

    auto *filterFrame = new QFrame(contentShell);
    filterFrame->setObjectName("videoFilter");
    auto *filterLayout = new QHBoxLayout(filterFrame);
    filterLayout->setContentsMargins(20, 12, 20, 12);
    filterLayout->setSpacing(10);
    auto *videosLabel = new QLabel("Replays", filterFrame);
    videosLabel->setObjectName("sectionTitle");
    filterLayout->addWidget(videosLabel);
    filterLayout->addStretch();

    struct FilterSpec {
        QString label;
        FilterMode mode;
    };
    const std::vector<FilterSpec> filterSpecs = {
        {"Grid", FilterMode::Grid},
        {"Drafts", FilterMode::Drafts},
        {"Tagged", FilterMode::Tagged}
    };

    auto *filterGroup = new QButtonGroup(filterFrame);
    filterGroup->setExclusive(true);
    filterButtons_.reserve(filterSpecs.size());
    for (const auto &spec : filterSpecs) {
        auto *tab = new QPushButton(spec.label, filterFrame);
        tab->setObjectName("filterTab");
        tab->setCheckable(true);
        tab->setCursor(Qt::PointingHandCursor);
        filterGroup->addButton(tab, static_cast<int>(spec.mode));
        filterLayout->addWidget(tab);
        filterButtons_.push_back(tab);

        QObject::connect(tab, &QPushButton::toggled, this, [this, index = static_cast<int>(spec.mode)](bool checked) {
            if (checked) {
                setActiveFilter(index);
            }
        });
    }

    gridFrame_ = new QFrame(contentShell);
    gridFrame_->setObjectName("videoGrid");
    gridLayout_ = new QGridLayout(gridFrame_);
    gridLayout_->setContentsMargins(18, 18, 18, 18);
    gridLayout_->setSpacing(14);
    emptyStateLabel_ = new QLabel(gridFrame_);
    emptyStateLabel_->setStyleSheet("color: #7b8fad; font-size: 16px;");
    emptyStateLabel_->setWordWrap(true);
    emptyStateLabel_->hide();

    const int total = std::min<int>(videos_.size(), 9);
    gridOrder_.reserve(total);
    draftOrder_.reserve((total + 1) / 2);
    taggedOrder_.reserve(total / 2);
    for (int i = 0; i < total; ++i) {
        gridOrder_.push_back(i);
        if (i % 2 == 0) {
            draftOrder_.push_back(i);
        } else {
            taggedOrder_.push_back(i);
        }
    }
    if (draftOrder_.empty() && !gridOrder_.empty()) {
        draftOrder_.push_back(gridOrder_.front());
    }
    if (taggedOrder_.empty() && gridOrder_.size() > 1) {
        taggedOrder_.push_back(gridOrder_.back());
    }

    if (!filterButtons_.empty()) {
        filterButtons_.front()->setChecked(true);
    } else {
        setActiveFilter(static_cast<int>(FilterMode::Grid));
    }

    shellLayout->addWidget(heroCard);
    shellLayout->addWidget(filterFrame);
    shellLayout->addWidget(gridFrame_);
    shellLayout->addStretch(1);

    contentLayout->addWidget(contentShell);
    contentLayout->addStretch(1);

    applyFollowState(false);
    rebuildGrid();
    updateResponsiveLayout();
}

void ProfilePage::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    updateResponsiveLayout();
}

void ProfilePage::updateResponsiveLayout() {
    if (!identityRow_ || !statsRow_ || !gridLayout_) {
        return;
    }

    const int w = width();
    const bool compactIdentity = w < 900;
    identityRow_->setDirection(compactIdentity ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight);
    statsRow_->setDirection(w < 720 ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight);

    int columns = 3;
    if (w < 580) {
        columns = 1;
    } else if (w < 980) {
        columns = 2;
    } else if (w >= 1400) {
        columns = 4;
    }

    if (videoTiles_.empty()) {
        return;
    }

    const int tileWidth = columns == 1 ? std::min(w - 120, 420) : columns == 4 ? 190 : 230;
    const int tileHeight = columns == 1 ? 360 : 300;

    for (int i = 0; i < static_cast<int>(videoTiles_.size()); ++i) {
        auto *tile = videoTiles_.at(i);
        tile->setFixedSize(tileWidth, tileHeight);
        tile->setIconSize(tile->size());
        gridLayout_->addWidget(tile, i / columns, i % columns);
    }
}

void ProfilePage::rebuildGrid() {
    if (!gridLayout_) {
        return;
    }

    for (auto *tile : videoTiles_) {
        gridLayout_->removeWidget(tile);
        tile->deleteLater();
    }
    videoTiles_.clear();

    if (emptyStateLabel_) {
        gridLayout_->removeWidget(emptyStateLabel_);
        emptyStateLabel_->hide();
    }

    const auto &order = orderFor(static_cast<int>(activeFilter_));
    if (order.empty()) {
        if (emptyStateLabel_) {
            emptyStateLabel_->setText(emptyStateFor(static_cast<int>(activeFilter_)));
            gridLayout_->addWidget(emptyStateLabel_, 0, 0, 1, 1);
            gridLayout_->setAlignment(emptyStateLabel_, Qt::AlignCenter);
            emptyStateLabel_->show();
        }
        return;
    }

    videoTiles_.reserve(order.size());
    for (int index : order) {
        if (index < 0 || index >= static_cast<int>(videos_.size())) {
            continue;
        }
        auto *tile = new QPushButton(gridFrame_);
        tile->setObjectName("videoTile");
        tile->setCursor(Qt::PointingHandCursor);
        tile->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        tile->setMinimumSize(180, 260);
        tile->setIconSize(QSize(180, 260));
        const auto &videoInfo = videos_.at(index);
        if (videoInfo.icon) {
            tile->setIcon(*videoInfo.icon);
        }
        QObject::connect(tile, &QPushButton::clicked, this, [this, index]() {
            emit playVideoRequested(index);
        });
        videoTiles_.push_back(tile);
    }

    updateResponsiveLayout();
}

void ProfilePage::setActiveFilter(int modeIndex) {
    const auto mode = static_cast<FilterMode>(modeIndex);
    if (activeFilter_ == mode) {
        return;
    }
    activeFilter_ = mode;
    rebuildGrid();
}

void ProfilePage::applyFollowState(bool following) {
    if (!followButton_) {
        return;
    }
    followButton_->setText(following ? "Following" : "Follow");
    if (!following && followButton_->isChecked()) {
        followButton_->blockSignals(true);
        followButton_->setChecked(false);
        followButton_->blockSignals(false);
    }
}

const std::vector<int> &ProfilePage::orderFor(int modeIndex) const {
    switch (static_cast<FilterMode>(modeIndex)) {
        case FilterMode::Grid:
            return gridOrder_;
        case FilterMode::Drafts:
            return draftOrder_;
        case FilterMode::Tagged:
            return taggedOrder_;
    }
    return gridOrder_;
}

QString ProfilePage::emptyStateFor(int modeIndex) const {
    switch (static_cast<FilterMode>(modeIndex)) {
        case FilterMode::Grid:
            return "No drops yet. Start sharing your world to fill this grid.";
        case FilterMode::Drafts:
            return "Drafts you save will land here before they are published.";
        case FilterMode::Tagged:
            return "When collaborators tag Lina, the replays will appear here.";
    }
    return {};
}


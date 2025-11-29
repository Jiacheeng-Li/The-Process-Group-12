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
#include <QConicalGradient>
#include <QPaintEvent>
#include <memory>
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

// 彩色头像环（仿 Instagram），使用项目配色做渐变
class AvatarRingWidget : public QWidget {
public:
    explicit AvatarRingWidget(const QIcon *icon, QWidget *parent = nullptr)
        : QWidget(parent) {
        if (icon) {
            icon_.reset(new QIcon(*icon));
        }
        setFixedSize(120, 120);
        setAttribute(Qt::WA_TranslucentBackground, true);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QRectF outerRect = rect().adjusted(1, 1, -1, -1);

        // 外圈：多色渐变环
        QConicalGradient grad(outerRect.center(), 0);
        // 多段渐变，让颜色衔接更顺滑，尽量减少纯黑
        grad.setColorAt(0.00, QColor("#FF4F70"));   // 粉
        grad.setColorAt(0.18, QColor("#FF8AA0"));   // 粉 → 亮一点
        grad.setColorAt(0.32, QColor("#6CADFF"));   // 淡蓝
        grad.setColorAt(0.46, QColor("#3A7DFF"));   // 深蓝
        grad.setColorAt(0.60, QColor("#BFBFBF"));   // 银灰
        grad.setColorAt(0.74, QColor("#6CADFF"));   // 回到淡蓝
        grad.setColorAt(0.88, QColor("#FF8AA0"));   // 再次粉色过渡
        grad.setColorAt(1.00, QColor("#FF4F70"));   // 闭环粉

        painter.setPen(Qt::NoPen);
        painter.setBrush(grad);
        painter.drawEllipse(outerRect);

        // 内圈：深色背景 + 头像
        const int ringWidth = 5;  // 加粗色环
        QRectF innerRect = outerRect.adjusted(ringWidth, ringWidth, -ringWidth, -ringWidth);
        // 内圈背景改成深蓝，而不是纯黑
        painter.setBrush(QColor("#050b1e"));
        painter.drawEllipse(innerRect);

        if (icon_) {
            const int d = static_cast<int>(qMin(innerRect.width(), innerRect.height()));
            QSize avatarSize(d, d);
            QPixmap avatarPixmap = roundedFromIcon(icon_.get(), avatarSize, d / 2);

            QPointF topLeft(innerRect.center().x() - avatarPixmap.width() / 2.0,
                            innerRect.center().y() - avatarPixmap.height() / 2.0);
            painter.drawPixmap(topLeft, avatarPixmap);
        }
    }

private:
    std::unique_ptr<QIcon> icon_;
};

QString defaultProfileStyle() {
    return QStringLiteral(
        "QWidget#profilePageRoot {"
        "  background: qradialgradient(cx:0.25, cy:0.2, radius:1.25,"
        "    stop:0 #091230, stop:0.5 #030918, stop:1 #00040a);"
        "}"
        "QScrollArea { background: transparent; border: none; }"
        "QScrollArea > QWidget > QWidget { background: transparent; }"
        "QFrame#contentShell {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(8,20,60,0.95), stop:1 rgba(12,40,118,0.9));"
        "  border-radius: 48px;"
        "  border: 2px solid rgba(58,125,255,0.45);"
        "  outline: 1px solid rgba(2,4,12,0.6);"
        "}"
        "QFrame#heroCard {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(8,20,60,0.98), stop:1 rgba(12,40,118,0.95));"
        "  border-radius: 32px;"
        "  border: 2px solid rgba(58,125,255,0.45);"
        "  outline: 1px solid rgba(2,4,12,0.6);"
        "}"
        "QHBoxLayout { background: transparent; }"
        "QVBoxLayout { background: transparent; }"
        "QLabel#displayName { font-size: 30px; font-weight: 700; color: white; }"
        "QLabel#username { font-size: 15px; color: #6f84a8; }"
        "QLabel#bioLabel { color: #cfd9ec; line-height: 1.6; }"
        "QLabel#statValue { font-size: 20px; font-weight: 700; color: white; }"
        "QLabel#statLabel { font-size: 12px; text-transform: uppercase; color: #6a819f; letter-spacing: 0.2em; }"
        "QPushButton#primaryCta {"
        "  background-color: #2f8dff;"
        "  border-radius: 22px;"
        "  padding: 12px 26px;"
        "  border: 2px solid rgba(76,162,255,0.8);"
        "  color: white;"
        "  font-weight: 700;"
        "  text-align: left;"
        "}"
        "QPushButton#primaryCta:hover {"
        "  background-color: #4ca2ff;"
        "  border-color: rgba(108,173,255,1.0);"
        "}"
        "QPushButton#primaryCta:checked {"
        "  background-color: rgba(22,37,66,0.85);"
        "  border: 2px solid rgba(76,162,255,0.8);"
        "  color: #9db6ff;"
        "}"
        "QPushButton#secondaryCta {"
        "  background-color: transparent;"
        "  color: white;"
        "  border: 1px solid rgba(76,162,255,0.5);"
        "  border-radius: 22px;"
        "  padding: 11px 24px;"
        "  font-weight: 600;"
        "  text-align: left;"
        "}"
        "QPushButton#secondaryCta:hover {"
        "  background-color: rgba(76,162,255,0.1);"
        "  border-color: rgba(76,162,255,0.8);"
        "}"
        "QFrame#videoFilter {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(8,20,60,0.98), stop:1 rgba(12,40,118,0.95));"
        "  border-radius: 20px;"
        "  border: 2px solid rgba(58,125,255,0.45);"
        "  outline: 1px solid rgba(2,4,12,0.6);"
        "}"
        "QFrame#videoGrid {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(8,20,60,0.98), stop:1 rgba(12,40,118,0.95));"
        "  border-radius: 30px;"
        "  border: 2px solid rgba(58,125,255,0.45);"
        "  outline: 1px solid rgba(2,4,12,0.6);"
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
}

QString highContrastProfileStyle() {
    return QStringLiteral(
        "QWidget#profilePageRoot { background: #0b0b0b; }"
        "QScrollArea { background: transparent; border: none; }"
        "QFrame#contentShell {"
        "  background: #1b1b1b;"
        "  border-radius: 48px;"
        "  border: 2px solid #f4c430;"
        "}"
        "QFrame#heroCard {"
        "  background: #0f0f0f;"
        "  border-radius: 32px;"
        "  border: 2px solid #ff9f1c;"
        "}"
        "QLabel#displayName { font-size: 30px; font-weight: 700; color: #ffffff; }"
        "QLabel#username { font-size: 15px; color: #f9f871; }"
        "QLabel#bioLabel { color: #ffffff; }"
        "QLabel#statValue { font-size: 20px; font-weight: 700; color: #ffffff; }"
        "QLabel#statLabel { font-size: 12px; text-transform: uppercase; color: #f4c430; letter-spacing: 0.2em; }"
        "QPushButton#primaryCta {"
        "  background-color: #ff9f1c;"
        "  border-radius: 22px;"
        "  padding: 12px 26px;"
        "  border: none;"
        "  color: #000;"
        "  font-weight: 700;"
        "}"
        "QPushButton#secondaryCta {"
        "  background-color: transparent;"
        "  color: #ffffff;"
        "  border: 2px solid #f4c430;"
        "  border-radius: 22px;"
        "  padding: 11px 24px;"
        "  font-weight: 600;"
        "}"
        "QFrame#videoFilter {"
        "  background: #0f0f0f;"
        "  border-radius: 20px;"
        "  border: 2px solid #f4c430;"
        "}"
        "QFrame#videoGrid {"
        "  background: #0f0f0f;"
        "  border-radius: 30px;"
        "  border: 2px solid #f4c430;"
        "  padding: 6px;"
        "}"
        "QPushButton#filterTab {"
        "  background-color: transparent;"
        "  color: #f4c430;"
        "  border: none;"
        "  font-weight: 600;"
        "  padding: 8px 16px;"
        "  border-radius: 14px;"
        "}"
        "QPushButton#filterTab:checked {"
        "  color: #000;"
        "  background-color: #f4c430;"
        "}"
        "QPushButton#videoTile {"
        "  background-color: #141414;"
        "  border: 2px solid #f4c430;"
        "  border-radius: 20px;"
        "  min-height: 260px;"
        "}"
        "QLabel#sectionTitle { font-size: 20px; font-weight: 700; color: #f9f871; }");
}

struct ProfileCopy {
    QString displayName;
    QString username;
    QString bio;
    QString follow;
    QString following;
    QString share;
    QString replays;
    QString filterGrid;
    QString filterDrafts;
    QString filterTagged;
    QString emptyGrid;
    QString emptyDrafts;
    QString emptyTagged;
    QString statFollowing;
    QString statFollowers;
    QString statLikes;
};

ProfileCopy localizedProfileCopy(AppLanguage language) {
    if (language == AppLanguage::English) {
        return {
            "Lina Mendes",
            "@linagoesreal",
            "Creator of rooftop BeReal drops • chasing golden hour on every continent • new lens drops every Sunday.",
            "Follow",
            "Following",
            "Share profile",
            "Replays",
            "Grid",
            "Drafts",
            "Tagged",
            "No drops yet. Start sharing your world to fill this grid.",
            "Drafts you save will land here before they are published.",
            "When collaborators tag Lina, the replays will appear here.",
            "Following",
            "Followers",
            "Likes"};
    }

    return {
        QString::fromUtf8("莉娜·门德斯"),
        QString::fromUtf8("@linagoesreal"),
        QString::fromUtf8("屋顶双摄创作者 · 追逐每一座城市的黄金时刻 · 每周日更新新镜头。"),
        QString::fromUtf8("关注"),
        QString::fromUtf8("已关注"),
        QString::fromUtf8("分享主页"),
        QString::fromUtf8("回放"),
        QString::fromUtf8("网格"),
        QString::fromUtf8("草稿"),
        QString::fromUtf8("被标记"),
        QString::fromUtf8("还没有内容，快去记录真实瞬间吧。"),
        QString::fromUtf8("保存的草稿会出现在这里。"),
        QString::fromUtf8("好友标记你后，合集会出现在这里。"),
        QString::fromUtf8("关注"),
        QString::fromUtf8("粉丝"),
        QString::fromUtf8("点赞")
    };
}

QWidget *createStatBadge(const QString &value,
                         const QString &label,
                         const QString &labelKey,
                         QWidget *parent,
                         std::vector<std::pair<QLabel *, QString>> *registry) {
    auto *wrapper = new QWidget(parent);
    auto *layout = new QVBoxLayout(wrapper);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *valueLabel = new QLabel(value, wrapper);
    valueLabel->setObjectName("statValue");
    auto *labelLabel = new QLabel(label, wrapper);
    labelLabel->setObjectName("statLabel");
    labelLabel->setProperty("statKey", labelKey);

    layout->addWidget(valueLabel, 0, Qt::AlignHCenter);
    layout->addWidget(labelLabel, 0, Qt::AlignHCenter);

    if (registry) {
        registry->push_back({labelLabel, labelKey});
    }

    return wrapper;
}
} // namespace

ProfilePage::ProfilePage(const std::vector<TheButtonInfo> &videos, QWidget *parent)
    : QWidget(parent),
      videos_(videos) {
    setObjectName("profilePageRoot");
    defaultStyleSheet_ = defaultProfileStyle();
    highContrastStyleSheet_ = highContrastProfileStyle();
    setStyleSheet(defaultStyleSheet_);

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

    // 使用彩色环头像
    const QIcon *avatarIcon = videos.empty() ? nullptr : videos.front().icon;
    auto *avatar = new AvatarRingWidget(avatarIcon, heroCard);

    auto *identityCol = new QVBoxLayout();
    identityCol->setSpacing(8);

    displayNameLabel_ = new QLabel("Lina Mendes", heroCard);
    displayNameLabel_->setObjectName("displayName");
    usernameLabel_ = new QLabel("@linagoesreal", heroCard);
    usernameLabel_->setObjectName("username");

    auto *ctaRow = new QHBoxLayout();
    ctaRow->setSpacing(12);
    auto *followBtn = new QPushButton("Follow", heroCard);
    followBtn->setObjectName("primaryCta");
    followBtn->setCheckable(true);
    followBtn->setCursor(Qt::PointingHandCursor);
    // 添加图标
    followBtn->setIcon(QIcon(":/icons/icons/follow.svg"));
    followBtn->setIconSize(QSize(22, 22));
    followButton_ = followBtn;
    QObject::connect(followBtn, &QPushButton::toggled, this, [this](bool checked) {
        applyFollowState(checked);
    });

    shareButton_ = new QPushButton("Share profile", heroCard);
    shareButton_->setObjectName("secondaryCta");
    // 添加图标
    shareButton_->setIcon(QIcon(":/icons/icons/share_profile.svg"));
    shareButton_->setIconSize(QSize(22, 22));
    ctaRow->addWidget(followBtn);
    ctaRow->addWidget(shareButton_);
    ctaRow->addStretch();

    identityCol->addWidget(displayNameLabel_);
    identityCol->addWidget(usernameLabel_);
    identityCol->addLayout(ctaRow);

    identityRow_->addWidget(avatar);
    identityRow_->addLayout(identityCol);
    identityRow_->addStretch();

    statsRow_ = new QHBoxLayout();
    statsRow_->setSpacing(24);
    statsRow_->addWidget(createStatBadge("428", "Following", "stat.following", heroCard, &statLabelWidgets_));
    statsRow_->addWidget(createStatBadge("18.3K", "Followers", "stat.followers", heroCard, &statLabelWidgets_));
    statsRow_->addWidget(createStatBadge("1.2M", "Likes", "stat.likes", heroCard, &statLabelWidgets_));
    statsRow_->addStretch();

    bioLabel_ = new QLabel("Creator of rooftop BeReal drops • chasing golden hour on every continent • new lens drops every Sunday.", heroCard);
    bioLabel_->setObjectName("bioLabel");
    bioLabel_->setWordWrap(true);

    heroLayout->addLayout(identityRow_);
    heroLayout->addLayout(statsRow_);
    heroLayout->addWidget(bioLabel_);

    auto *filterFrame = new QFrame(contentShell);
    filterFrame->setObjectName("videoFilter");
    auto *filterLayout = new QHBoxLayout(filterFrame);
    filterLayout->setContentsMargins(20, 12, 20, 12);
    filterLayout->setSpacing(10);
    videosLabel_ = new QLabel("Replays", filterFrame);
    videosLabel_->setObjectName("sectionTitle");
    filterLayout->addWidget(videosLabel_);
    filterLayout->addStretch();

    struct FilterSpec {
        QString label;
        FilterMode mode;
        QString key;
    };
    const std::vector<FilterSpec> filterSpecs = {
        {"Grid", FilterMode::Grid, "grid"},
        {"Drafts", FilterMode::Drafts, "drafts"},
        {"Tagged", FilterMode::Tagged, "tagged"}
    };

    auto *filterGroup = new QButtonGroup(filterFrame);
    filterGroup->setExclusive(true);
    filterButtons_.reserve(filterSpecs.size());
    for (const auto &spec : filterSpecs) {
        auto *tab = new QPushButton(spec.label, filterFrame);
        tab->setObjectName("filterTab");
        tab->setCheckable(true);
        tab->setCursor(Qt::PointingHandCursor);
        tab->setProperty("filterKey", spec.key);
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

    rebuildStrings();
    updateResponsiveLayout();
}

void ProfilePage::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    updateResponsiveLayout();
}

void ProfilePage::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制彩色边框（和Homepage一样）
    // 外层矩形略微内缩，避免被裁剪
    QRectF outerRect = rect().adjusted(3.0, 3.0, -3.0, -3.0);
    
    // 使用和头像类似的多色渐变做一圈「色环边框」
    QConicalGradient grad(outerRect.center(), 0);
    grad.setColorAt(0.00, QColor("#FF4F70"));
    grad.setColorAt(0.18, QColor("#FF8AA0"));
    grad.setColorAt(0.32, QColor("#6CADFF"));
    grad.setColorAt(0.46, QColor("#3A7DFF"));
    grad.setColorAt(0.60, QColor("#BFBFBF"));
    grad.setColorAt(0.74, QColor("#6CADFF"));
    grad.setColorAt(0.88, QColor("#FF8AA0"));
    grad.setColorAt(1.00, QColor("#FF4F70"));
    
    QPen pen(QBrush(grad), 5.0);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    
    const qreal radius = 10.0;
    painter.drawRoundedRect(outerRect, radius, radius);
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
    followButton_->setText(following ? followingLabel_ : followLabel_);
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
            return emptyGridLabel_;
        case FilterMode::Drafts:
            return emptyDraftsLabel_;
        case FilterMode::Tagged:
            return emptyTaggedLabel_;
    }
    return {};
}

void ProfilePage::setLanguage(AppLanguage language) {
    if (language_ == language) {
        rebuildStrings();
        return;
    }
    language_ = language;
    rebuildStrings();
}

void ProfilePage::rebuildStrings() {
    const ProfileCopy copy = localizedProfileCopy(language_);
    displayNameText_ = copy.displayName;
    usernameText_ = copy.username;
    bioText_ = copy.bio;
    followLabel_ = copy.follow;
    followingLabel_ = copy.following;
    shareLabel_ = copy.share;
    replaysLabel_ = copy.replays;
    gridLabel_ = copy.filterGrid;
    draftsLabel_ = copy.filterDrafts;
    taggedLabel_ = copy.filterTagged;
    emptyGridLabel_ = copy.emptyGrid;
    emptyDraftsLabel_ = copy.emptyDrafts;
    emptyTaggedLabel_ = copy.emptyTagged;

    if (displayNameLabel_) {
        displayNameLabel_->setText(displayNameText_);
    }
    if (usernameLabel_) {
        usernameLabel_->setText(usernameText_);
    }
    if (bioLabel_) {
        bioLabel_->setText(bioText_);
    }
    if (shareButton_) {
        shareButton_->setText(shareLabel_);
    }
    if (videosLabel_) {
        videosLabel_->setText(replaysLabel_);
    }
    for (auto *button : filterButtons_) {
        const QString key = button->property("filterKey").toString();
        if (key == "grid") {
            button->setText(gridLabel_);
        } else if (key == "drafts") {
            button->setText(draftsLabel_);
        } else if (key == "tagged") {
            button->setText(taggedLabel_);
        }
    }

    for (auto &entry : statLabelWidgets_) {
        if (!entry.first) {
            continue;
        }
        if (entry.second == "stat.following") {
            entry.first->setText(copy.statFollowing);
        } else if (entry.second == "stat.followers") {
            entry.first->setText(copy.statFollowers);
        } else if (entry.second == "stat.likes") {
            entry.first->setText(copy.statLikes);
        }
    }

    applyFollowState(followButton_ && followButton_->isChecked());
    rebuildGrid();
}

void ProfilePage::setHighContrastMode(bool enabled) {
    if (highContrastMode_ == enabled) {
        return;
    }
    highContrastMode_ = enabled;
    setStyleSheet(enabled ? highContrastStyleSheet_ : defaultStyleSheet_);
}


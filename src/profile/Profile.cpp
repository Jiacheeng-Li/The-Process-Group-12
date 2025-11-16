#include "Profile.h"
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QFont>
#include <QStyleOption>

// ====================== CircleAvatar 实现 ======================
CircleAvatar::CircleAvatar(QWidget *parent, int size)
    : QWidget(parent), m_size(size) {
    setFixedSize(m_size, m_size);
    setStyleSheet("background-color: #f0f0f0; border-radius: 50%;");
}

void CircleAvatar::setAvatar(const QString &path) {
    m_avatarPath = path;
    update(); // 触发重绘
}

void CircleAvatar::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 抗锯齿

    // 绘制圆形遮罩
    QPainterPath path;
    path.addEllipse(rect().adjusted(2, 2, -2, -2)); // 内缩2px，避免边框溢出
    painter.setClipPath(path);

    // 绘制头像
    if (!m_avatarPath.isEmpty()) {
        QPixmap pixmap(m_avatarPath);
        if (!pixmap.isNull()) {
            // 按比例缩放并平滑显示
            painter.drawPixmap(rect().adjusted(2, 2, -2, -2),
                               pixmap.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        }
    } else {
        // 默认灰色头像
        painter.setBrush(QColor(200, 200, 200));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(rect().adjusted(2, 2, -2, -2));
    }
}

// ====================== StatCard 实现 ======================
StatCard::StatCard(const QString &title, int count, QWidget *parent)
    : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    // 数字标签（加粗放大）
    m_countLabel = new QLabel(QString::number(count));
    QFont countFont = m_countLabel->font();
    countFont.setBold(true);
    countFont.setPointSize(16);
    m_countLabel->setFont(countFont);
    m_countLabel->setAlignment(Qt::AlignCenter);

    // 标题标签（灰色小字体）
    m_titleLabel = new QLabel(title);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(12);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("color: #666666;");

    layout->addWidget(m_countLabel);
    layout->addWidget(m_titleLabel);
}

// ====================== VideoCard 实现 ======================
VideoCard::VideoCard(const QString &coverPath, QWidget *parent)
    : QWidget(parent) {
    setMinimumSize(120, 210); // 视频卡片宽高比（9:16）
    setStyleSheet("background-color: #000000; border-radius: 8px;");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // 视频封面标签
    m_coverLabel = new QLabel(this);
    m_coverLabel->setStyleSheet("border-radius: 8px;");
    m_coverLabel->setScaledContents(true); // 自动缩放图片填充
    layout->addWidget(m_coverLabel);

    // 设置封面（优先自定义，无则默认）
    if (!coverPath.isEmpty()) {
        QPixmap pixmap(coverPath);
        if (!pixmap.isNull()) {
            m_coverLabel->setPixmap(pixmap);
        } else {
            setDefaultCover();
        }
    } else {
        setDefaultCover();
    }
}

void VideoCard::setDefaultCover() {
    // 生成默认封面（黑色背景+白色播放图标）
    QPixmap defaultCover(120, 210);
    defaultCover.fill(Qt::black);

    QPainter painter(&defaultCover);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制三角形播放图标
    QPointF points[3] = {
        QPointF(45, 85),
        QPointF(75, 105),
        QPointF(45, 125)
    };
    painter.setBrush(Qt::white);
    painter.setPen(Qt::NoPen);
    painter.drawConvexPolygon(points, 3);

    m_coverLabel->setPixmap(defaultCover);
}

// ====================== TikTokProfile 实现 ======================
TikTokProfile::TikTokProfile(QWidget *parent)
    : QWidget(parent) {
    initUI();
    setWindowTitle("TikTok 个人主页");
    setMinimumSize(800, 600); // 最小窗口尺寸
}

void TikTokProfile::initUI() {
    // 主布局（垂直排列）
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    // ---------------------- 1. 个人信息区域 ----------------------
    QHBoxLayout *profileLayout = new QHBoxLayout();
    profileLayout->setSpacing(30);

    // 头像
    m_avatar = new CircleAvatar(this, 120);
    // 替换为实际图片路径（本地文件或资源文件）
    m_avatar->setAvatar(":/avatar.png");
    profileLayout->addWidget(m_avatar);

    // 右侧信息（垂直布局）
    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(15);

    // 用户名 + 功能按钮（水平布局）
    QHBoxLayout *userActionLayout = new QHBoxLayout();
    userActionLayout->setSpacing(15);

    // 用户名
    QLabel *usernameLabel = new QLabel("@tiktok_creator");
    QFont usernameFont = usernameLabel->font();
    usernameFont.setBold(true);
    usernameFont.setPointSize(18);
    usernameLabel->setFont(usernameFont);
    userActionLayout->addWidget(usernameLabel);

    // 关注按钮
    QPushButton *followBtn = new QPushButton("Follow");
    followBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #000000;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 6px 16px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #333333;
        }
    )");
    userActionLayout->addWidget(followBtn);

    // 私信按钮
    QPushButton *messageBtn = new QPushButton("Text");
    messageBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #f0f0f0;
            color: #000000;
            border: none;
            border-radius: 8px;
            padding: 6px 16px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #e0e0e0;
        }
    )");
    userActionLayout->addWidget(messageBtn);

    // 更多按钮
    QPushButton *moreBtn = new QPushButton("⋮");
    moreBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #f0f0f0;
            color: #000000;
            border: none;
            border-radius: 8px;
            padding: 6px 10px;
            font-size: 18px;
        }
        QPushButton:hover {
            background-color: #e0e0e0;
        }
    )");
    userActionLayout->addWidget(moreBtn);

    infoLayout->addLayout(userActionLayout);

    // 统计数据（作品/粉丝/关注）
    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(30);
    statsLayout->addWidget(new StatCard("Videos", 24));
    statsLayout->addWidget(new StatCard("Follower", 15600));
    statsLayout->addWidget(new StatCard("Following", 320));
    infoLayout->addLayout(statsLayout);

    // 个人简介
    QLabel *bioLabel = new QLabel("Creator who livs a loving life\n Mailing：contact@example.com");
    bioLabel->setStyleSheet("font-size: 14px; color: #333333;");
    bioLabel->setWordWrap(true); // 自动换行
    infoLayout->addWidget(bioLabel);

    profileLayout->addLayout(infoLayout);
    profileLayout->addStretch(); // 右侧拉伸，使信息靠左对齐
    mainLayout->addLayout(profileLayout);

    // ---------------------- 2. 内容切换栏 ----------------------
    QWidget *tabWidget = new QWidget();
    QHBoxLayout *tabLayout = new QHBoxLayout(tabWidget);
    tabLayout->setContentsMargins(0, 0, 0, 0);
    tabLayout->setSpacing(30);

    // 视频标签（当前选中）
    QPushButton *videoTabBtn = new QPushButton("Videos");
    videoTabBtn->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            color: #000000;
            border: none;
            border-bottom: 2px solid #000000;
            padding: 8px 0;
            font-size: 16px;
            font-weight: bold;
        }
    )");
    tabLayout->addWidget(videoTabBtn);

    // 喜欢标签（未选中）
    QPushButton *likedTabBtn = new QPushButton("Liked");
    likedTabBtn->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            color: #666666;
            border: none;
            padding: 8px 0;
            font-size: 16px;
        }
        QPushButton:hover {
            color: #000000;
        }
    )");
    tabLayout->addWidget(likedTabBtn);

    mainLayout->addWidget(tabWidget);

    // ---------------------- 3. 视频展示区（滚动） ----------------------
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: none; }");

    // 视频容器（网格布局）
    QWidget *videoContainer = new QWidget();
    QGridLayout *videoLayout = new QGridLayout(videoContainer);
    videoLayout->setContentsMargins(0, 0, 0, 0);
    videoLayout->setSpacing(20);

    // 添加9个示例视频卡片（3列布局）
    for (int i = 0; i < 9; ++i) {
        VideoCard *videoCard = new VideoCard();
        int row = i / 3;
        int col = i % 3;
        videoLayout->addWidget(videoCard, row, col);
    }

    scrollArea->setWidget(videoContainer);
    mainLayout->addWidget(scrollArea, 1); // 占满剩余空间
}

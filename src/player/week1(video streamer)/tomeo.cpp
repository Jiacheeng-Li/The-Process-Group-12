//
//    ______
//   /_  __/___  ____ ___  ___  ____
//    / / / __ \/ __ `__ \/ _ \/ __ \
//   / / / /_/ / / / / / /  __/ /_/ /
//  /_/  \____/_/ /_/ /_/\___/\____/
//              video for sports enthusiasts...
//              Mobile Version - Portrait Mode
//

#include <iostream>
#include <QApplication>
#include <QtMultimediaWidgets/QVideoWidget>
#include <QMediaPlaylist>
#include <string>
#include <vector>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtCore/QFileInfo>
#include <QtWidgets/QFileIconProvider>
#include <QDesktopServices>
#include <QImageReader>
#include <QMessageBox>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QStackedWidget>
#include <QScrollArea>
#include <QScroller>
#include <QMouseEvent>
#include "the_player.h"
#include "the_button.h"

// 自定义视频容器，支持点击切换
class VideoContainer : public QWidget {
    Q_OBJECT
public:
    VideoContainer(QWidget *parent = nullptr) : QWidget(parent) {}

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override {
        emit clicked();
        QWidget::mousePressEvent(event);
    }
};

// 创建圆形按钮
QPushButton* createRoundButton(const QString &iconText, QWidget *parent) {
    QPushButton *btn = new QPushButton(iconText, parent);
    btn->setFixedSize(50, 50);
    btn->setStyleSheet(
        "QPushButton {"
        "   background-color: rgba(50, 50, 50, 180);"
        "   color: white;"
        "   border-radius: 25px;"
        "   font-size: 20px;"
        "   border: 2px solid rgba(255, 255, 255, 100);"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(80, 80, 80, 200);"
        "}"
        "QPushButton:pressed {"
        "   background-color: rgba(100, 100, 100, 220);"
        "}"
        );
    return btn;
}

// 创建导航栏按钮
QPushButton* createNavButton(const QString &text, const QString &icon, QWidget *parent) {
    QPushButton *btn = new QPushButton(icon + "\n" + text, parent);
    btn->setFixedSize(70, 60);
    btn->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent;"
        "   color: white;"
        "   border: none;"
        "   font-size: 10px;"
        "}"
        "QPushButton:hover {"
        "   color: #00D9FF;"
        "}"
        );
    return btn;
}

// read in videos and thumbnails to this directory
std::vector<TheButtonInfo> getInfoIn (std::string loc) {

    std::vector<TheButtonInfo> out =  std::vector<TheButtonInfo>();
    QDir dir(QString::fromStdString(loc) );
    QDirIterator it(dir);

    qDebug() << "Scanning directory:" << QString::fromStdString(loc) << Qt::endl;

    while (it.hasNext()) { // for all files

        QString f = it.next();

        if (f.contains("."))

#if defined(_WIN32)
        if (f.contains(".wmv"))  { // windows
#else
        if (f.contains(".mp4") || f.contains(".MOV"))  { // mac/linux
#endif

            QString thumb = f.left( f .length() - 4) +".png";
            qDebug() << "Found video:" << f << Qt::endl;
            qDebug() << "Looking for thumbnail:" << thumb << Qt::endl;

            if (QFile(thumb).exists()) { // if a png thumbnail exists
                QImageReader *imageReader = new QImageReader(thumb);
                QImage sprite = imageReader->read(); // read the thumbnail
                if (!sprite.isNull()) {
                    QIcon* ico = new QIcon(QPixmap::fromImage(sprite)); // voodoo to create an icon for the button
                    QUrl* url = new QUrl(QUrl::fromLocalFile( f )); // convert the file location to a generic url
                    out . push_back(TheButtonInfo( url , ico  ) ); // add to the output list
                    qDebug() << "Successfully added video:" << f << Qt::endl;
                }
                else
                    qDebug() << "warning: skipping video because I couldn't process thumbnail " << thumb << Qt::endl;
            }
            else
                qDebug() << "warning: skipping video because I couldn't find thumbnail " << thumb << Qt::endl;
        }
    }

    qDebug() << "Total videos found:" << out.size() << Qt::endl;
    return out;
}


int main(int argc, char *argv[]) {

    // let's just check that Qt is operational first
    qDebug() << "Qt version: " << QT_VERSION_STR << Qt::endl;

    // create the Qt Application
    QApplication app(argc, argv);

    // collect all the videos in the folder
    std::vector<TheButtonInfo> videos;

    if (argc == 2) {
        qDebug() << "Video folder argument:" << argv[1] << Qt::endl;
        videos = getInfoIn( std::string(argv[1]) );
    } else {
        qDebug() << "No command line argument provided!" << Qt::endl;
    }

    if (videos.size() == 0) {

        const int result = QMessageBox::information(
            NULL,
            QString("Tomeo"),
            QString("No videos found!\n\nPlease:\n1. Add command line argument with video folder path\n2. Make sure videos have .mp4 or .MOV extension\n3. Make sure each video has a matching .png thumbnail"));
        exit(-1);
    }

    qDebug() << "Creating player with" << videos.size() << "videos" << Qt::endl;

    // 创建主窗口
    QWidget window;
    window.setWindowTitle("Tomeo - Mobile");
    window.setFixedSize(360, 640);  // 竖屏尺寸
    window.setStyleSheet("background-color: black;");

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(&window);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ==================== 创建视频播放器（放在最前面）====================
    QVideoWidget *videoWidget = new QVideoWidget(&window);
    videoWidget->setStyleSheet("background-color: black;");
    videoWidget->setAspectRatioMode(Qt::KeepAspectRatio);

    ThePlayer *player = new ThePlayer;
    player->setVideoOutput(videoWidget);

    // ==================== 视频列表（创建按钮连接）====================
    std::vector<TheButton*> buttons;
    for ( int i = 0; i < std::min(10, (int)videos.size()); i++ ) {
        TheButton *button = new TheButton(&window);
        button->connect(button, SIGNAL(jumpTo(TheButtonInfo* )),
                        player, SLOT (jumpTo(TheButtonInfo*)));
        buttons.push_back(button);
        button->init(&videos.at(i));
        button->hide(); // 隐藏按钮，因为我们用点击切换
    }

    player->setContent(&buttons, &videos);

    qDebug() << "Player initialized, starting first video..." << Qt::endl;

    // ==================== 视频区域容器 ====================
    VideoContainer *videoContainer = new VideoContainer(&window);
    videoContainer->setStyleSheet("background-color: black;");

    // 使用简单的布局，避免复杂的堆叠
    QVBoxLayout *videoContainerLayout = new QVBoxLayout(videoContainer);
    videoContainerLayout->setContentsMargins(0, 0, 0, 0);
    videoContainerLayout->setSpacing(0);

    // 创建一个容器来放置视频和覆盖层
    QWidget *videoWithOverlay = new QWidget(videoContainer);
    videoWithOverlay->setStyleSheet("background-color: black;");

    // 先添加视频widget到一个简单布局
    QVBoxLayout *videoBaseLayout = new QVBoxLayout(videoWithOverlay);
    videoBaseLayout->setContentsMargins(0, 0, 0, 0);
    videoBaseLayout->addWidget(videoWidget);

    // ==================== 顶部用户信息栏 ====================
    QWidget *topBar = new QWidget(videoWithOverlay);
    topBar->setFixedHeight(60);
    topBar->setStyleSheet(
        "background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 rgba(0, 0, 0, 200), stop:1 rgba(0, 0, 0, 0));"
        );
    topBar->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    topBar->raise(); // 置于最上层

    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(10, 10, 10, 10);

    // 用户头像
    QLabel *avatar = new QLabel(topBar);
    avatar->setFixedSize(40, 40);
    avatar->setStyleSheet(
        "background-color: #FF6B9D;"
        "border-radius: 20px;"
        "border: 2px solid white;"
        );
    avatar->setAlignment(Qt::AlignCenter);
    QFont avatarFont;
    avatarFont.setPointSize(16);
    avatarFont.setBold(true);
    avatar->setFont(avatarFont);
    avatar->setText("👤");

    // 用户名
    QLabel *username = new QLabel("@用户名", topBar);
    username->setStyleSheet("color: white; font-size: 14px; font-weight: bold;");

    // 关注按钮
    QPushButton *followBtn = new QPushButton("+ 关注", topBar);
    followBtn->setFixedSize(60, 30);
    followBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #FF6B9D;"
        "   color: white;"
        "   border-radius: 15px;"
        "   font-size: 12px;"
        "   font-weight: bold;"
        "   border: none;"
        "}"
        "QPushButton:hover {"
        "   background-color: #FF8AB5;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #E05A8C;"
        "}"
        );

    topLayout->addWidget(avatar);
    topLayout->addWidget(username);
    topLayout->addStretch();
    topLayout->addWidget(followBtn);

    // 设置topBar的位置
    topBar->setGeometry(0, 0, 360, 60);

    // ==================== 右侧悬浮按钮栏 ====================
    QWidget *rightBar = new QWidget(videoWithOverlay);
    rightBar->setFixedWidth(70);
    rightBar->setStyleSheet("background-color: transparent;");
    rightBar->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    rightBar->raise(); // 置于最上层

    QVBoxLayout *rightLayout = new QVBoxLayout(rightBar);
    rightLayout->setContentsMargins(10, 0, 10, 20);
    rightLayout->addStretch();

    // 点赞按钮
    QPushButton *likeBtn = createRoundButton("❤️", rightBar);
    QLabel *likeCount = new QLabel("12.5K", rightBar);
    likeCount->setAlignment(Qt::AlignCenter);
    likeCount->setStyleSheet("color: white; font-size: 11px;");

    // 评论按钮
    QPushButton *commentBtn = createRoundButton("💬", rightBar);
    QLabel *commentCount = new QLabel("3.2K", rightBar);
    commentCount->setAlignment(Qt::AlignCenter);
    commentCount->setStyleSheet("color: white; font-size: 11px;");

    // 分享按钮
    QPushButton *shareBtn = createRoundButton("↗️", rightBar);
    QLabel *shareCount = new QLabel("分享", rightBar);
    shareCount->setAlignment(Qt::AlignCenter);
    shareCount->setStyleSheet("color: white; font-size: 11px;");

    rightLayout->addWidget(likeBtn);
    rightLayout->addWidget(likeCount);
    rightLayout->addSpacing(15);
    rightLayout->addWidget(commentBtn);
    rightLayout->addWidget(commentCount);
    rightLayout->addSpacing(15);
    rightLayout->addWidget(shareBtn);
    rightLayout->addWidget(shareCount);
    rightLayout->addSpacing(30);

    // 设置rightBar的位置（右下角）
    rightBar->setGeometry(290, 370, 70, 200);

    videoContainerLayout->addWidget(videoWithOverlay);

    // ==================== 底部导航栏 ====================
    QWidget *bottomNav = new QWidget(&window);
    bottomNav->setFixedHeight(70);
    bottomNav->setStyleSheet(
        "background-color: rgba(20, 20, 20, 250);"
        "border-top: 1px solid rgba(255, 255, 255, 50);"
        );

    QHBoxLayout *navLayout = new QHBoxLayout(bottomNav);
    navLayout->setContentsMargins(5, 5, 5, 5);
    navLayout->setSpacing(0);

    QPushButton *homeBtn = createNavButton("首页", "🏠", bottomNav);
    QPushButton *communityBtn = createNavButton("社区", "👥", bottomNav);
    QPushButton *cameraBtn = createNavButton("", "📷", bottomNav);
    cameraBtn->setFixedSize(60, 60);
    cameraBtn->setStyleSheet(
        "QPushButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "   stop:0 #FF6B9D, stop:1 #FFA07A);"
        "   color: white;"
        "   border-radius: 30px;"
        "   font-size: 24px;"
        "   border: 3px solid rgba(20, 20, 20, 250);"
        "}"
        "QPushButton:hover {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "   stop:0 #FF8AB5, stop:1 #FFB89A);"
        "}"
        );
    QPushButton *messageBtn = createNavButton("消息", "✉️", bottomNav);
    QPushButton *profileBtn = createNavButton("我的", "👤", bottomNav);

    // 设置首页按钮为选中状态
    homeBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent;"
        "   color: #00D9FF;"
        "   border: none;"
        "   font-size: 10px;"
        "}"
        );

    navLayout->addWidget(homeBtn);
    navLayout->addWidget(communityBtn);
    navLayout->addWidget(cameraBtn);
    navLayout->addWidget(messageBtn);
    navLayout->addWidget(profileBtn);

    // ==================== 组装主布局 ====================
    mainLayout->addWidget(videoContainer, 1);
    mainLayout->addWidget(bottomNav);

    // ==================== 交互功能 ====================
    int currentVideoIndex = 0;

    // 点击视频区域切换到下一个视频
    QObject::connect(videoContainer, &VideoContainer::clicked, [&]() {
        currentVideoIndex = (currentVideoIndex + 1) % videos.size();
        player->jumpTo(&videos.at(currentVideoIndex));
        qDebug() << "Switched to video" << currentVideoIndex;
    });

    // 点赞按钮功能
    bool isLiked = false;
    QObject::connect(likeBtn, &QPushButton::clicked, [&]() {
        isLiked = !isLiked;
        if (isLiked) {
            likeBtn->setStyleSheet(
                "QPushButton {"
                "   background-color: rgba(255, 50, 100, 200);"
                "   color: white;"
                "   border-radius: 25px;"
                "   font-size: 20px;"
                "   border: 2px solid rgba(255, 255, 255, 100);"
                "}"
                );
            likeCount->setText("12.6K");
        } else {
            likeBtn->setStyleSheet(
                "QPushButton {"
                "   background-color: rgba(50, 50, 50, 180);"
                "   color: white;"
                "   border-radius: 25px;"
                "   font-size: 20px;"
                "   border: 2px solid rgba(255, 255, 255, 100);"
                "}"
                );
            likeCount->setText("12.5K");
        }
    });

    // 评论按钮功能
    QObject::connect(commentBtn, &QPushButton::clicked, [&]() {
        QMessageBox::information(&window, "评论", "评论功能待开发...");
    });

    // 分享按钮功能
    QObject::connect(shareBtn, &QPushButton::clicked, [&]() {
        QMessageBox::information(&window, "分享", "分享功能待开发...");
    });

    // 关注按钮功能
    bool isFollowed = false;
    QObject::connect(followBtn, &QPushButton::clicked, [&]() {
        isFollowed = !isFollowed;
        if (isFollowed) {
            followBtn->setText("✓ 已关注");
            followBtn->setStyleSheet(
                "QPushButton {"
                "   background-color: rgba(100, 100, 100, 150);"
                "   color: white;"
                "   border-radius: 15px;"
                "   font-size: 12px;"
                "   font-weight: bold;"
                "   border: 1px solid rgba(255, 255, 255, 100);"
                "}"
                );
        } else {
            followBtn->setText("+ 关注");
            followBtn->setStyleSheet(
                "QPushButton {"
                "   background-color: #FF6B9D;"
                "   color: white;"
                "   border-radius: 15px;"
                "   font-size: 12px;"
                "   font-weight: bold;"
                "   border: none;"
                "}"
                );
        }
    });

    // showtime!
    window.show();

    // wait for the app to terminate
    return app.exec();
}

// 需要包含 moc 生成的文件
#include "tomeo.moc"

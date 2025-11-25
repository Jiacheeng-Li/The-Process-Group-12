//
//    ______
//   /_  __/___  ____ ___  ___  ____
//    / / / __ \/ __ `__ \/ _ \/ __ \
//   / / / /_/ / / / / / /  __/ /_/ /
//  /_/  \____/_/ /_/ /_/\___/\____/
//              video for sports enthusiasts...
//
//

#include <iostream>
#include <QApplication>
#include <QtMultimediaWidgets/QVideoWidget>
#include <QMediaPlaylist>
#include <string>
#include <vector>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QFrame>
#include <QtCore/QFileInfo>
#include <QtWidgets/QFileIconProvider>
#include <QtCore/QFile>
#include <QDesktopServices>
#include <QImageReader>
#include <QMessageBox>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include "the_player.h"
#include "the_button.h"
#include "profile_page.h"
#include "chat_page.h"
#include "recordpage.h"
#include "publishpage.h"
#include "friendspage.h"
#include <QStackedWidget>
#include <QButtonGroup>
#include <QGraphicsDropShadowEffect>
#include <QMenu>
#include <QEvent>
#include <QBoxLayout>
#include <QStackedLayout>
#include <functional>
#include <QSizePolicy>
#include <QSlider>
#include <QTimer>
#include <QTime>
#include <QtGlobal>
#include <algorithm>
#include <limits>

class ResizeWatcher : public QObject {
public:
    using Callback = std::function<void(QWidget *)>;

    ResizeWatcher(QWidget *target, Callback callback)
        : QObject(target), target_(target), callback_(std::move(callback)) {
        if (target_) {
            target_->installEventFilter(this);
            if (callback_) {
                callback_(target_);
            }
        }
    }

protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (obj == target_ && event->type() == QEvent::Resize && callback_) {
            callback_(target_);
        }
        return QObject::eventFilter(obj, event);
    }

private:
    QWidget *target_ = nullptr;
    Callback callback_;
};

// read in videos and thumbnails from this directory
std::vector<TheButtonInfo> getInfoIn(std::string loc) {
    std::vector<TheButtonInfo> out;

    QDir dir(QString::fromStdString(loc));
    if (!dir.exists()) {
        qWarning() << "Video directory does not exist:" << dir.absolutePath();
        return out;
    }

    QDirIterator it(dir);
    while (it.hasNext()) {
        const QString f = it.next();
        QFileInfo fi(f);
        if (!fi.isFile()) {
            continue;
        }

        const QString lower = f.toLower();
        bool isVideo = false;

#if defined(_WIN32)
        // Windows: prefer wmv / mp4
        if (lower.endsWith(".wmv") || lower.endsWith(".mp4")) {
            isVideo = true;
        }
#else
        // macOS / Linux: mp4 / mov
        if (lower.endsWith(".mp4") || lower.endsWith(".mov")) {
            isVideo = true;
        }
#endif

        if (!isVideo) {
            continue;
        }

        // Try to load thumbnail if it exists; otherwise fall back to no icon
        QIcon *ico = nullptr;
        const QString thumb = f.left(f.length() - 4) + ".png";
        if (QFile::exists(thumb)) {
            QImageReader imageReader(thumb);
            const QImage sprite = imageReader.read();
            if (!sprite.isNull()) {
                ico = new QIcon(QPixmap::fromImage(sprite));
            } else {
                qDebug() << "warning: skipping thumbnail (failed to read):" << thumb;
            }
        } else {
            qDebug() << "info: no thumbnail found for" << f << "- using plain tile";
        }

        QUrl *url = new QUrl(QUrl::fromLocalFile(f));
        if (!url->isLocalFile() || !QFile::exists(url->toLocalFile())) {
            qWarning() << "Skipping video with invalid path:" << url->toString();
            delete url;
            delete ico;
            continue;
        }

        qDebug() << "Discovered video:" << url->toLocalFile();
        out.push_back(TheButtonInfo(url, ico));
    }

    if (out.empty()) {
        qWarning() << "No playable videos found in directory:" << dir.absolutePath();
    }

    return out;
}


int main(int argc, char *argv[]) {

    // let's just check that Qt is operational first
    qDebug() << "Qt version: " << QT_VERSION_STR;

    // create the Qt Application
    QApplication app(argc, argv);

    // collect all the videos in the folder
    std::vector<TheButtonInfo> videos;

    if (argc == 2)
        videos = getInfoIn( std::string(argv[1]) );

    if (videos.size() == 0) {

        const int result = QMessageBox::information(
            NULL,
            QString("Tomeo"),
            QString("no videos found! Add command line argument to \"quoted\" file location."));
        exit(-1);
    }

    // the widget that will show the video
    QVideoWidget *videoWidget = new QVideoWidget;
    videoWidget->setObjectName("homeVideo");
    videoWidget->setMinimumHeight(420);

    // the QMediaPlayer which controls the playback
    ThePlayer *player = new ThePlayer;
    player->setVideoOutput(videoWidget);
    player->setContent(nullptr, &videos);

    QWidget *homePage = new QWidget();
    homePage->setObjectName("homePage");
    QVBoxLayout *homeLayout = new QVBoxLayout();
    homeLayout->setContentsMargins(48, 48, 48, 48);
    homeLayout->setSpacing(24);
    homePage->setLayout(homeLayout);

    auto *topBar = new QHBoxLayout();
    topBar->setContentsMargins(0, 0, 0, 0);
    topBar->setSpacing(12);

    auto *titleCol = new QVBoxLayout();
    titleCol->setSpacing(4);
    auto *heroTitle = new QLabel(QString::fromUtf8("Today on BeReal Earth"));
    heroTitle->setObjectName("heroTitle");
    auto *heroSubtitle = new QLabel(QString::fromUtf8("双摄延迟 %1 min · 与朋友共享未修饰瞬间").arg(8));
    heroSubtitle->setObjectName("heroSubtitle");
    titleCol->addWidget(heroTitle);
    titleCol->addWidget(heroSubtitle);

    topBar->addLayout(titleCol);
    topBar->addStretch();

    auto *settingsButton = new QPushButton("Settings", homePage);
    settingsButton->setObjectName("settingsButton");
    settingsButton->setCursor(Qt::PointingHandCursor);
    topBar->addWidget(settingsButton, 0, Qt::AlignRight);

    homeLayout->addLayout(topBar);

    auto *berealCard = new QFrame(homePage);
    berealCard->setObjectName("berealCard");
    berealCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto *cardLayout = new QVBoxLayout(berealCard);
    cardLayout->setContentsMargins(28, 28, 28, 24);
    cardLayout->setSpacing(18);

    auto *cardHeader = new QHBoxLayout();
    cardHeader->setSpacing(16);

    auto *avatar = new QLabel(berealCard);
    avatar->setObjectName("berealAvatar");
    avatar->setFixedSize(52, 52);
    avatar->setScaledContents(true);
    if (!videos.empty() && videos.front().icon) {
        avatar->setPixmap(videos.front().icon->pixmap(avatar->size()));
    } else {
        avatar->setStyleSheet("background-color: rgba(255,255,255,0.15); border-radius: 26px;");
    }

    auto *identityCol = new QVBoxLayout();
    identityCol->setSpacing(2);
    auto *displayName = new QLabel("Lina Mendes", berealCard);
    displayName->setObjectName("displayName");
    auto *dropMeta = new QLabel(QString::fromUtf8("2 小时 late · Palermo, Buenos Aires"), berealCard);
    dropMeta->setObjectName("dropMeta");
    identityCol->addWidget(displayName);
    identityCol->addWidget(dropMeta);

    auto *momentLabel = new QLabel(videos.empty()
                                       ? QString::fromUtf8("Real drop · 0 / 0")
                                       : QString::fromUtf8("Real drop · 1 / %1").arg(videos.size()),
                                   berealCard);
    momentLabel->setObjectName("momentLabel");

    cardHeader->addWidget(avatar, 0, Qt::AlignTop);
    cardHeader->addLayout(identityCol);
    cardHeader->addStretch();
    cardHeader->addWidget(momentLabel, 0, Qt::AlignTop);

    cardLayout->addLayout(cardHeader);

    auto *captureFrame = new QFrame(berealCard);
    captureFrame->setObjectName("captureFrame");
    captureFrame->setMinimumSize(360, 460);
    captureFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto *captureStack = new QStackedLayout(captureFrame);
    captureStack->setStackingMode(QStackedLayout::StackAll);
    captureStack->setContentsMargins(0, 0, 0, 0);
    captureStack->addWidget(videoWidget);

    auto *overlayLayer = new QWidget(captureFrame);
    overlayLayer->setObjectName("captureOverlay");
    overlayLayer->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    auto *overlayLayout = new QVBoxLayout(overlayLayer);
    overlayLayout->setContentsMargins(18, 18, 18, 18);
    overlayLayout->setSpacing(12);

    auto *overlayTop = new QHBoxLayout();
    overlayTop->setSpacing(8);
    auto *lateBadge = new QLabel(QString::fromUtf8("双摄延迟 2h"), overlayLayer);
    lateBadge->setObjectName("lateBadge");
    auto *networkBadge = new QLabel(QString::fromUtf8("4G · 68%"), overlayLayer);
    networkBadge->setObjectName("networkBadge");
    overlayTop->addWidget(lateBadge);
    overlayTop->addStretch();
    overlayTop->addWidget(networkBadge);
    overlayLayout->addLayout(overlayTop);
    overlayLayout->addStretch();

    auto *overlayBottom = new QHBoxLayout();
    overlayBottom->setSpacing(12);
    overlayBottom->addStretch();
    auto *videoStatusLabel = new QLabel(QString::fromUtf8("加载中..."), overlayLayer);
    videoStatusLabel->setObjectName("videoStatusLabel");
    videoStatusLabel->setAlignment(Qt::AlignCenter);
    videoStatusLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    videoStatusLabel->hide();
    overlayBottom->addWidget(videoStatusLabel, 0, Qt::AlignBottom);
    overlayBottom->addStretch();
    auto *selfieBubble = new QLabel(QString::fromUtf8("自拍"), overlayLayer);
    selfieBubble->setObjectName("selfieBubble");
    selfieBubble->setAlignment(Qt::AlignCenter);
    selfieBubble->setScaledContents(true);
    overlayBottom->addWidget(selfieBubble, 0, Qt::AlignBottom);
    overlayLayout->addLayout(overlayBottom);

    captureStack->addWidget(overlayLayer);

    cardLayout->addWidget(captureFrame);

    auto *metaFooter = new QFrame(berealCard);
    metaFooter->setObjectName("metaFooter");
    auto *metaLayout = new QHBoxLayout(metaFooter);
    metaLayout->setContentsMargins(0, 0, 0, 0);
    metaLayout->setSpacing(12);
    auto *timeLabel = new QLabel(QString::fromUtf8("捕捉时间 · 16:42"), metaFooter);
    timeLabel->setObjectName("metaLabel");
    auto *locationLabel = new QLabel(QString::fromUtf8("位置 · Palermo Rooftop"), metaFooter);
    locationLabel->setObjectName("metaLabel");
    metaLayout->addWidget(timeLabel);
    metaLayout->addWidget(locationLabel);
    metaLayout->addStretch();
    auto *shareNowButton = new QPushButton(QString::fromUtf8("同步到好友"), metaFooter);
    shareNowButton->setObjectName("shareNowButton");
    metaLayout->addWidget(shareNowButton);
    cardLayout->addWidget(metaFooter);

    auto *progressSlider = new QSlider(Qt::Horizontal, berealCard);
    progressSlider->setObjectName("progressSlider");
    progressSlider->setRange(0, 0);
    progressSlider->setEnabled(!videos.empty());
    progressSlider->setTracking(false);  // 禁用跟踪，只在释放时更新
    cardLayout->addWidget(progressSlider);
    
    // 用于跟踪用户是否正在拖动进度条
    auto *isDragging = new bool(false);

    auto *controlRow = new QHBoxLayout();
    controlRow->setSpacing(12);

    auto makePillButton = [&](const QString &label, bool checkable = false) {
        auto *btn = new QPushButton(label, berealCard);
        btn->setObjectName("pillButton");
        btn->setCursor(Qt::PointingHandCursor);
        btn->setCheckable(checkable);
        return btn;
    };

    auto *playPauseButton = makePillButton(QString::fromUtf8("暂停"));
    auto *muteButton = makePillButton(QString::fromUtf8("静音"), true);
    auto *prevMomentButton = makePillButton(QString::fromUtf8("上一瞬间"));
    auto *nextMomentButton = makePillButton(QString::fromUtf8("下一瞬间"));
    auto *retakeButton = makePillButton(QString::fromUtf8("重拍提示"));

    controlRow->addWidget(playPauseButton);
    controlRow->addWidget(muteButton);
    controlRow->addWidget(prevMomentButton);
    controlRow->addWidget(nextMomentButton);
    controlRow->addStretch();
    controlRow->addWidget(retakeButton);

    cardLayout->addLayout(controlRow);

    auto *reactionRow = new QHBoxLayout();
    reactionRow->setSpacing(8);
    auto *reactionPrompt = new QLabel(QString::fromUtf8("好友的实时反应"), berealCard);
    reactionPrompt->setObjectName("reactionPrompt");
    reactionRow->addWidget(reactionPrompt);
    reactionRow->addStretch();
    auto makeReactionButton = [&](const QString &emoji) {
        auto *btn = new QPushButton(emoji, berealCard);
        btn->setObjectName("reactionButton");
        btn->setCursor(Qt::PointingHandCursor);
        return btn;
    };
    std::vector<QPushButton *> reactionButtons = {
        makeReactionButton("😍"),
        makeReactionButton("🔥"),
        makeReactionButton("👏"),
        makeReactionButton("😂")
    };
    for (auto *btn : reactionButtons) {
        reactionRow->addWidget(btn);
    }
    cardLayout->addLayout(reactionRow);

    auto *commentPanel = new QFrame(berealCard);
    commentPanel->setObjectName("commentPanel");
    auto *commentLayout = new QVBoxLayout(commentPanel);
    commentLayout->setContentsMargins(18, 18, 18, 18);
    commentLayout->setSpacing(8);
    auto *captionLabel = new QLabel(QString::fromUtf8("“暮色刚刚落在屋顶，我们同时按下快门。”"), commentPanel);
    captionLabel->setObjectName("captionLabel");
    captionLabel->setWordWrap(true);
    auto *commentLabel = new QLabel(QString::fromUtf8("Luca: 这景色像极了我们上次的深夜实验！"), commentPanel);
    commentLabel->setObjectName("commentLabel");
    commentLabel->setWordWrap(true);
    auto *replyButton = new QPushButton(QString::fromUtf8("回复好友"), commentPanel);
    replyButton->setObjectName("replyButton");
    replyButton->setCursor(Qt::PointingHandCursor);
    commentLayout->addWidget(captionLabel);
    commentLayout->addWidget(commentLabel);
    commentLayout->addWidget(replyButton, 0, Qt::AlignRight);
    cardLayout->addWidget(commentPanel);

    homeLayout->addWidget(berealCard, 0, Qt::AlignTop);
    homeLayout->addStretch();

    int currentVideoIndex = 0;
    auto setVideoStatus = [videoStatusLabel](const QString &text, bool visible) {
        if (!videoStatusLabel) {
            return;
        }
        if (!text.isEmpty()) {
            videoStatusLabel->setText(text);
        }
        videoStatusLabel->setVisible(visible);
    };

    auto updateMomentMeta = [&](int index) {
        if (videos.empty()) {
            momentLabel->setText(QString::fromUtf8("Real drop · 0 / 0"));
            captionLabel->setText(QString::fromUtf8("暂无回忆"));
            return;
        }
        const QFileInfo fileInfo(videos.at(index).url->toLocalFile());
        const QString clipName = fileInfo.completeBaseName().isEmpty()
                                     ? QString::fromUtf8("未命名片段")
                                     : fileInfo.completeBaseName();
        momentLabel->setText(QString::fromUtf8("Real drop · %1 / %2").arg(index + 1).arg(videos.size()));
        captionLabel->setText(QString::fromUtf8("“%1 · 与好友同步”").arg(clipName));
        timeLabel->setText(QString::fromUtf8("捕捉时间 · %1").arg(QTime::currentTime().toString("hh:mm")));
        locationLabel->setText(QString::fromUtf8("位置 · Palermo Rooftop"));
        reactionPrompt->setText(QString::fromUtf8("好友的实时反应"));
    };

    auto updateSelfie = [&](int index) {
        if (!selfieBubble) {
            return;
        }
        if (index >= 0 && index < static_cast<int>(videos.size()) && videos.at(index).icon) {
            const QPixmap pix = videos.at(index).icon->pixmap(selfieBubble->size());
            selfieBubble->setPixmap(pix);
            selfieBubble->setText("");
        } else {
            selfieBubble->setPixmap(QPixmap());
            selfieBubble->setText(QString::fromUtf8("自拍"));
        }
    };

    auto playVideoAt = [&](int index) {
        if (videos.empty()) {
            return;
        }
        if (index < 0) {
            index = static_cast<int>(videos.size()) - 1;
        }
        currentVideoIndex = index % static_cast<int>(videos.size());
        setVideoStatus(QString::fromUtf8("加载中..."), true);
        updateMomentMeta(currentVideoIndex);
        updateSelfie(currentVideoIndex);
        player->setMedia(*videos.at(currentVideoIndex).url);
        player->play();
    };

    QObject::connect(nextMomentButton, &QPushButton::clicked, [&, playVideoAt]() {
        playVideoAt(currentVideoIndex + 1);
    });
    QObject::connect(prevMomentButton, &QPushButton::clicked, [&, playVideoAt]() {
        playVideoAt(currentVideoIndex - 1);
    });

    QObject::connect(retakeButton, &QPushButton::clicked, [setVideoStatus]() {
        setVideoStatus(QString::fromUtf8("贴士：BeReal 只允许一次重拍"), true);
        QTimer::singleShot(2200, [setVideoStatus]() {
            setVideoStatus(QString(), false);
        });
    });

    QObject::connect(shareNowButton, &QPushButton::clicked, [&]() {
        QMessageBox::information(berealCard, QString::fromUtf8("同步完成"),
                                 QString::fromUtf8("你的双摄瞬间已经推送给好友 ✅"));
    });

    QObject::connect(replyButton, &QPushButton::clicked, [&]() {
        QMessageBox::information(berealCard, QString::fromUtf8("回复好友"),
                                 QString::fromUtf8("准备发一条“真实”评论吧！"));
    });

    QObject::connect(playPauseButton, &QPushButton::clicked, [player, playPauseButton]() {
        if (player->state() == QMediaPlayer::PlayingState) {
            player->pause();
        } else {
            player->play();
        }
    });

    QObject::connect(player, &QMediaPlayer::stateChanged, [playPauseButton](QMediaPlayer::State state) {
        if (state == QMediaPlayer::PlayingState) {
            playPauseButton->setText(QString::fromUtf8("暂停"));
        } else {
            playPauseButton->setText(QString::fromUtf8("播放"));
        }
    });

    QObject::connect(muteButton, &QPushButton::toggled, [player, muteButton](bool checked) {
        player->setMuted(checked);
        muteButton->setText(checked ? QString::fromUtf8("取消静音") : QString::fromUtf8("静音"));
    });

    // 进度条拖动处理
    QObject::connect(progressSlider, &QSlider::sliderPressed, [isDragging]() {
        *isDragging = true;
    });
    QObject::connect(progressSlider, &QSlider::sliderMoved, [player, isDragging](int value) {
        if (*isDragging) {
            player->setPosition(value);
        }
    });
    QObject::connect(progressSlider, &QSlider::sliderReleased, [player, progressSlider, isDragging]() {
        *isDragging = false;
        player->setPosition(progressSlider->value());
    });

    QObject::connect(player, &QMediaPlayer::durationChanged, [progressSlider](qint64 duration) {
        const int sliderMax = duration > 0
                                  ? static_cast<int>(std::min<qint64>(duration, std::numeric_limits<int>::max()))
                                  : 0;
        progressSlider->setMaximum(sliderMax);
        progressSlider->setEnabled(sliderMax > 0);
    });
    QObject::connect(player, &QMediaPlayer::positionChanged, [progressSlider, isDragging](qint64 position) {
        // 只有在不是用户拖动的时候才更新进度条位置
        if (!(*isDragging) && !progressSlider->isSliderDown()) {
            const int sliderPos = static_cast<int>(std::min<qint64>(position, std::numeric_limits<int>::max()));
            progressSlider->setValue(sliderPos);
        }
    });

    for (auto *btn : reactionButtons) {
        QObject::connect(btn, &QPushButton::clicked, [reactionPrompt, emoji = btn->text()]() {
            reactionPrompt->setText(QString::fromUtf8("你刚刚发送了 %1").arg(emoji));
            QTimer::singleShot(2000, [reactionPrompt]() {
                reactionPrompt->setText(QString::fromUtf8("好友的实时反应"));
            });
        });
    }

    new ResizeWatcher(captureFrame, [videoWidget, overlayLayer, selfieBubble](QWidget *frame) {
        const int width = frame->width();
        int targetHeight = static_cast<int>(width * 4.0 / 3.0);
        targetHeight = qBound(420, targetHeight, 900);
        frame->setFixedHeight(targetHeight);
        if (videoWidget) {
            videoWidget->setFixedSize(frame->size());
        }
        if (overlayLayer) {
            overlayLayer->setFixedSize(frame->size());
        }
        if (selfieBubble) {
            const int bubbleWidth = qMax(110, width / 4);
            const int bubbleHeight = static_cast<int>(bubbleWidth * 4.0 / 3.0);
            selfieBubble->setFixedSize(bubbleWidth, bubbleHeight);
        }
    });

    playVideoAt(0);

    QObject::connect(player, &QMediaPlayer::mediaStatusChanged,
                     [setVideoStatus](QMediaPlayer::MediaStatus status) {
                         switch (status) {
                         case QMediaPlayer::LoadingMedia:
                         case QMediaPlayer::BufferedMedia:
                         case QMediaPlayer::StalledMedia:
                             setVideoStatus(QString::fromUtf8("加载中..."), true);
                             break;
                         case QMediaPlayer::EndOfMedia:
                             setVideoStatus(QString::fromUtf8("播放结束"), true);
                             break;
                         case QMediaPlayer::LoadedMedia:
                             setVideoStatus(QString(), false);
                             break;
                         default:
                             break;
                         }
                     });

    QObject::connect(player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error),
                     [player, setVideoStatus](QMediaPlayer::Error error) {
                         if (error == QMediaPlayer::NoError) {
                             return;
                         }
                         const QString errText = player->errorString().isEmpty()
                                                     ? QString::fromUtf8("无法播放此视频")
                                                     : player->errorString();
                         setVideoStatus(QString::fromUtf8("播放失败: %1").arg(errText), true);
                     });

    QObject::connect(player, &QMediaPlayer::stateChanged, [setVideoStatus](QMediaPlayer::State state) {
        if (state == QMediaPlayer::PlayingState) {
            setVideoStatus(QString(), false);
        }
    });

    ProfilePage *profilePage = new ProfilePage(videos);
    ChatPage *chatPage = new ChatPage();
    FriendsPage *friendsPage = new FriendsPage(videos);  // 传递视频列表
    RecordPage *recordPage = new RecordPage();
    PublishPage *publishPage = new PublishPage();

    QStackedWidget *stackedPages = new QStackedWidget();
    stackedPages->addWidget(homePage);      // 0: Home
    stackedPages->addWidget(friendsPage);   // 1: Friends
    stackedPages->addWidget(recordPage);    // 2: Record
    stackedPages->addWidget(chatPage);      // 3: Chat
    stackedPages->addWidget(profilePage);   // 4: Profile
    stackedPages->addWidget(publishPage);   // 5: Publish (隐藏页面，通过信号跳转)

    QWidget window;
    window.setObjectName("appRoot");
    QVBoxLayout *top = new QVBoxLayout();
    top->setContentsMargins(28, 24, 28, 32);
    top->setSpacing(16);
    window.setLayout(top);
    window.setWindowTitle("tomeo");
    window.setMinimumSize(420, 720);

    const QString backgroundUrl(":/home/earth.png");

    auto buildNightStyle = [&](const QString &bg) {
        return QString(
                   "QWidget#appRoot {"
                   "  background-color: #00040d;"
                   "}"
                   "QWidget#homePage {"
                   "  background-color: rgba(1,3,10,0.2);"
                   "  background-image: url(%1);"
                   "  background-repeat: no-repeat;"
                   "  background-position: center center;"
                   "  background-size: contain;"
                   "  background-attachment: fixed;"
                   "}"
                   "QLabel#heroTitle { font-size: 22px; font-weight: 700; color: white; }"
                   "QLabel#heroSubtitle { color: #6f84b8; }"
                   "QPushButton#settingsButton {"
                   "  background-color: rgba(15,30,55,0.85);"
                   "  color: white;"
                   "  border: 1px solid rgba(63,134,255,0.35);"
                   "  border-radius: 22px;"
                   "  padding: 10px 22px;"
                   "  font-weight: 600;"
                   "}"
                   "QPushButton#settingsButton:hover { background-color: rgba(59,124,220,0.65); }"
                   "QFrame#berealCard {"
                   "  background: rgba(2,8,20,0.92);"
                   "  border-radius: 40px;"
                   "  border: 1px solid rgba(63,134,255,0.35);"
                   "  outline: 1px solid rgba(2,4,12,0.6);"
                   "}"
                   "QLabel#displayName { color: white; font-size: 18px; font-weight: 700; }"
                   "QLabel#dropMeta { color: #8aa7d9; }"
                   "QLabel#momentLabel { color: #9db6ff; font-weight: 600; }"
                   "QFrame#captureFrame { background: black; border-radius: 32px; }"
                   "QVideoWidget#homeVideo { border-radius: 32px; background-color: black; }"
                   "QWidget#captureOverlay { border-radius: 32px; }"
                   "QLabel#lateBadge {"
                   "  background-color: rgba(255,255,255,0.12);"
                   "  color: #93caff;"
                   "  border-radius: 18px;"
                   "  padding: 6px 14px;"
                   "  font-weight: 600;"
                   "}"
                   "QLabel#networkBadge { color: #8aa7d9; }"
                   "QLabel#videoStatusLabel {"
                   "  background-color: rgba(0,0,0,0.55);"
                   "  color: white;"
                   "  font-size: 16px;"
                   "  font-weight: 600;"
                   "  padding: 10px 24px;"
                   "  border-radius: 24px;"
                   "}"
                   "QLabel#selfieBubble {"
                   "  background-color: rgba(5,10,22,0.85);"
                   "  color: white;"
                   "  border: 2px solid rgba(157,182,255,0.4);"
                   "  border-radius: 28px;"
                   "  font-weight: 600;"
                   "}"
                   "QFrame#metaFooter { color: #8aa7d9; }"
                   "QLabel#metaLabel { color: #8aa7d9; }"
                   "QPushButton#shareNowButton {"
                   "  background-color: rgba(59,124,220,0.9);"
                   "  color: white;"
                   "  border: none;"
                   "  border-radius: 20px;"
                   "  padding: 8px 20px;"
                   "  font-weight: 600;"
                   "}"
                   "QSlider#progressSlider::groove {"
                   "  height: 4px;"
                   "  background: rgba(255,255,255,0.18);"
                   "  border-radius: 2px;"
                   "}"
                   "QSlider#progressSlider::handle {"
                   "  width: 16px;"
                   "  background: rgba(255,255,255,0.9);"
                   "  border-radius: 8px;"
                   "  margin: -6px 0;"
                   "}"
                   "QSlider#progressSlider::sub-page {"
                   "  background: rgba(93,155,255,0.85);"
                   "  border-radius: 2px;"
                   "}"
                   "QPushButton#pillButton {"
                   "  background-color: rgba(24,72,156,0.85);"
                   "  color: white;"
                   "  border: none;"
                   "  border-radius: 18px;"
                   "  padding: 10px 18px;"
                   "  font-weight: 600;"
                   "}"
                   "QPushButton#pillButton:hover { background-color: rgba(59,124,220,0.95); }"
                   "QPushButton#pillButton:checked { background-color: rgba(255,255,255,0.12); color: #9db6ff; }"
                   "QLabel#reactionPrompt { color: #8aa7d9; }"
                   "QPushButton#reactionButton {"
                   "  background-color: rgba(6,16,34,0.9);"
                   "  color: white;"
                   "  border: 1px solid rgba(157,182,255,0.3);"
                   "  border-radius: 16px;"
                   "  padding: 6px 12px;"
                   "  font-size: 18px;"
                   "}"
                   "QFrame#commentPanel {"
                   "  background: rgba(4,12,26,0.95);"
                   "  border-radius: 26px;"
                   "  border: 1px solid rgba(63,134,255,0.3);"
                   "}"
                   "QLabel#captionLabel { color: white; font-size: 16px; font-weight: 600; }"
                   "QLabel#commentLabel { color: #9fb1d6; }"
                   "QPushButton#replyButton {"
                   "  background: transparent;"
                   "  color: #9db6ff;"
                   "  border: none;"
                   "  font-weight: 600;"
                   "}"
                   "QFrame#floatingNav {"
                   "  background-color: rgba(4,10,20,0.92);"
                   "  border-radius: 28px;"
                   "  border: 1px solid rgba(47,141,255,0.25);"
                   "}"
                   "QPushButton#navButton {"
                   "  color: #6e85b8;"
                   "  background: transparent;"
                   "  border: none;"
                   "  font-size: 14px;"
                   "  font-weight: 600;"
                   "  padding: 12px 18px;"
                   "  border-radius: 18px;"
                   "}"
                   "QPushButton#navButton:hover { color: #9db6ff; }"
            "QPushButton#navButton:checked {"
                   "  color: white;"
                   "  background-color: rgba(47,141,255,0.2);"
                   "}"
        ).arg(bg);
    };

    auto buildDayStyle = [&](const QString &bg) {
        return QString(
            "QWidget#appRoot {"
            "  background-color: #f4f7ff;"
            "}"
                   "QWidget#homePage {"
            "  background-color: rgba(231,237,247,0.2);"
            "  background-image: url(%1);"
            "  background-repeat: no-repeat;"
            "  background-position: center center;"
            "  background-size: contain;"
            "  background-attachment: fixed;"
                   "}"
                   "QLabel#heroTitle { font-size: 22px; font-weight: 700; color: #0c1b33; }"
                   "QLabel#heroSubtitle { color: #4f5f7f; }"
                   "QPushButton#settingsButton {"
                   "  background-color: rgba(255,255,255,0.95);"
                   "  color: #20324f;"
                   "  border: 1px solid rgba(58,82,132,0.25);"
                   "  border-radius: 22px;"
                   "  padding: 10px 22px;"
                   "  font-weight: 600;"
                   "}"
                   "QFrame#berealCard {"
                   "  background: rgba(255,255,255,0.92);"
                   "  border-radius: 40px;"
                   "  border: 1px solid rgba(88,118,178,0.2);"
                   "}"
                   "QLabel#displayName { color: #0c1b33; }"
                   "QLabel#dropMeta { color: #5f6d8c; }"
                   "QLabel#momentLabel { color: #3353b3; font-weight: 600; }"
                   "QFrame#captureFrame { background: #000; border-radius: 32px; }"
                   "QVideoWidget#homeVideo { border-radius: 32px; }"
                   "QLabel#lateBadge { background-color: rgba(255,255,255,0.85); color: #2f4ea7; }"
                   "QLabel#networkBadge { color: #5f6d8c; }"
                   "QLabel#videoStatusLabel { background-color: rgba(12,18,40,0.55); color: white; }"
                   "QLabel#selfieBubble {"
                   "  background-color: rgba(255,255,255,0.9);"
                   "  color: #20324f;"
                   "  border: 2px solid rgba(58,82,132,0.25);"
                   "  border-radius: 28px;"
                   "}"
                   "QFrame#metaFooter { color: #5f6d8c; }"
                   "QLabel#metaLabel { color: #5f6d8c; }"
                   "QPushButton#shareNowButton {"
                   "  background-color: #3353b3;"
                   "  color: white;"
                   "  border: none;"
                   "  border-radius: 20px;"
                   "  padding: 8px 20px;"
                   "  font-weight: 600;"
                   "}"
                   "QSlider#progressSlider::groove { background: rgba(32,50,90,0.15); height: 4px; border-radius: 2px; }"
                   "QSlider#progressSlider::handle { width: 16px; background: #3353b3; border-radius: 8px; margin: -6px 0; }"
                   "QSlider#progressSlider::sub-page { background: #6f8dff; border-radius: 2px; }"
                   "QPushButton#pillButton {"
                   "  background-color: rgba(14,28,72,0.08);"
                   "  color: #0c1b33;"
                   "  border: 1px solid rgba(12,27,51,0.08);"
                   "  border-radius: 18px;"
                   "  padding: 10px 18px;"
                   "  font-weight: 600;"
                   "}"
                   "QPushButton#pillButton:hover { background-color: rgba(51,83,179,0.15); }"
                   "QPushButton#pillButton:checked { background-color: rgba(51,83,179,0.2); color: #20324f; }"
                   "QLabel#reactionPrompt { color: #5f6d8c; }"
                   "QPushButton#reactionButton {"
                   "  background-color: rgba(255,255,255,0.95);"
                   "  color: #20324f;"
                   "  border: 1px solid rgba(58,82,132,0.2);"
                   "  border-radius: 16px;"
                   "  padding: 6px 12px;"
                   "  font-size: 18px;"
                   "}"
                   "QFrame#commentPanel {"
                   "  background: rgba(255,255,255,0.92);"
                   "  border-radius: 26px;"
                   "  border: 1px solid rgba(88,118,178,0.2);"
                   "}"
                   "QLabel#captionLabel { color: #0c1b33; }"
                   "QLabel#commentLabel { color: #4f5f7f; }"
                   "QPushButton#replyButton {"
                   "  background: transparent;"
                   "  color: #3353b3;"
                   "  border: none;"
                   "  font-weight: 600;"
                   "}"
                   "QFrame#floatingNav {"
                   "  background-color: rgba(255,255,255,0.92);"
                   "  border-radius: 28px;"
                   "  border: 1px solid rgba(58,82,132,0.2);"
                   "}"
                   "QPushButton#navButton {"
                   "  color: #51658c;"
                   "  background: transparent;"
                   "  border: none;"
                   "  font-size: 14px;"
                   "  font-weight: 600;"
                   "  padding: 12px 18px;"
                   "  border-radius: 18px;"
                   "}"
                   "QPushButton#navButton:hover { color: #20324f; }"
            "QPushButton#navButton:checked {"
                   "  color: white;"
                   "  background-color: rgba(51,83,179,0.8);"
                   "}"
        ).arg(bg);
    };

    bool nightMode = true;
    auto applyTheme = [&](bool night) {
        nightMode = night;
        window.setStyleSheet(night ? buildNightStyle(backgroundUrl) : buildDayStyle(backgroundUrl));
    };
    applyTheme(true);

    top->addWidget(stackedPages, 1);

    auto *navWrapper = new QWidget(&window);
    auto *navWrapperLayout = new QHBoxLayout(navWrapper);
    navWrapperLayout->setContentsMargins(0, 0, 0, 0);
    navWrapperLayout->addStretch();

    auto *floatingNav = new QFrame(navWrapper);
    floatingNav->setObjectName("floatingNav");
    floatingNav->setFixedHeight(84);
    auto *floatingLayout = new QHBoxLayout(floatingNav);
    floatingLayout->setContentsMargins(20, 12, 20, 12);
    floatingLayout->setSpacing(12);

    auto *shadow = new QGraphicsDropShadowEffect(floatingNav);
    shadow->setBlurRadius(36);
    shadow->setOffset(0, 10);
    shadow->setColor(QColor(0, 0, 0, 180));
    floatingNav->setGraphicsEffect(shadow);

    QButtonGroup *navGroup = new QButtonGroup(&window);
    navGroup->setExclusive(true);

    QPushButton *profileNavButton = nullptr;
    QPushButton *homeNavButton = nullptr;
    QPushButton *friendsNavButton = nullptr;
    QPushButton *recordNavButton = nullptr;
    QPushButton *chatNavButton = nullptr;

    struct NavSpec {
        QString label;
        int index;
    };
    const std::vector<NavSpec> navSpecs = {
        {QString::fromUtf8("主页"), 0},      // Home
        {QString::fromUtf8("朋友圈"), 1},    // Friends
        {QString::fromUtf8("录制"), 2},      // Record
        {QString::fromUtf8("聊天"), 3},      // Chat
        {QString::fromUtf8("个人"), 4}       // Profile
    };

    QPushButton *firstButton = nullptr;
    for (const auto &spec : navSpecs) {
        auto *button = new QPushButton(spec.label, floatingNav);
        button->setObjectName("navButton");
        button->setCheckable(true);
        button->setCursor(Qt::PointingHandCursor);
        navGroup->addButton(button, spec.index);
        floatingLayout->addWidget(button);

        const int targetIndex = spec.index;
        QObject::connect(button, &QPushButton::toggled, stackedPages, [stackedPages, targetIndex](bool checked) {
            if (checked) {
                stackedPages->setCurrentIndex(targetIndex);
            }
        });

        // 处理导航栏按钮引用
        if (targetIndex == 0) {
            homeNavButton = button;
        } else if (targetIndex == 1) {
            friendsNavButton = button;
        } else if (targetIndex == 2) {
            recordNavButton = button;
        } else if (targetIndex == 3) {
            chatNavButton = button;
        } else if (targetIndex == 4) {
            profileNavButton = button;
        }

        if (!firstButton) {
            firstButton = button;
        }
    }

    if (firstButton) {
        firstButton->setChecked(true);
    }

    navWrapperLayout->addWidget(floatingNav, 0, Qt::AlignBottom);
    navWrapperLayout->addStretch();

    new ResizeWatcher(&window, [floatingLayout, floatingNav](QWidget *root) {
        const bool compact = root->width() < 640;
        floatingLayout->setDirection(compact ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight);
        floatingLayout->setSpacing(compact ? 6 : 12);
        floatingNav->setFixedHeight(compact ? 120 : 84);
    });

    top->addWidget(navWrapper, 0, Qt::AlignBottom);

    // 页面跳转逻辑：Record -> Publish
    QObject::connect(recordPage, &RecordPage::recordingFinished, [&](){
        stackedPages->setCurrentIndex(5);  // 跳转到 Publish 页面
    });

    // 页面跳转逻辑：Publish -> Friends
    QObject::connect(publishPage, &PublishPage::sendPressed,
                     [&](const QString &thumb){
                         friendsPage->addNewPost(thumb);
                         if (friendsNavButton) {
                             friendsNavButton->setChecked(true);
                         } else {
                             stackedPages->setCurrentIndex(1);  // Friends 页面
                         }
                     });

    // 页面跳转逻辑：Publish -> Record (返回)
    QObject::connect(publishPage, &PublishPage::backToRecord, [&](){
        if (recordNavButton) {
            recordNavButton->setChecked(true);
        } else {
            stackedPages->setCurrentIndex(2);  // Record 页面
        }
    });

    // 页面跳转逻辑：Record -> Publish (选择草稿)
    QObject::connect(recordPage, &RecordPage::draftSelected, [&](const QString &draftText){
        publishPage->loadDraft(draftText);
        stackedPages->setCurrentIndex(5);  // 跳转到 Publish 页面
    });

    // Profile 页面播放视频请求
    if (profilePage) {
        QObject::connect(profilePage, &ProfilePage::playVideoRequested, &window, [&, homeNavButton](int index) {
            if (homeNavButton && !homeNavButton->isChecked()) {
                homeNavButton->setChecked(true);
            } else {
                stackedPages->setCurrentIndex(0);
            }
            playVideoAt(index);
        });
    }

    // Friends 页面跳转到 Profile（如果需要）
    QObject::connect(friendsPage, &FriendsPage::goToProfile, [&](const QString &username) {
        if (profileNavButton) {
            profileNavButton->setChecked(true);
        } else {
            stackedPages->setCurrentIndex(4);  // Profile 页面
        }
    });

    if (settingsButton) {
        QMenu *settingsMenu = new QMenu(settingsButton);
        QAction *dayModeAction = settingsMenu->addAction(QString::fromUtf8("日间模式"));
        QAction *nightModeAction = settingsMenu->addAction(QString::fromUtf8("夜间模式"));

        QObject::connect(dayModeAction, &QAction::triggered, [applyTheme]() mutable {
            applyTheme(false);
        });
        QObject::connect(nightModeAction, &QAction::triggered, [applyTheme]() mutable {
            applyTheme(true);
        });

        QObject::connect(settingsButton, &QPushButton::clicked, [settingsMenu, settingsButton]() {
            settingsMenu->exec(settingsButton->mapToGlobal(QPoint(settingsButton->width(), settingsButton->height())));
        });
    }

    // showtime!
    window.show();

    // wait for the app to terminate
    return app.exec();
}













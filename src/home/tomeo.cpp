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
#include <QtCore/QDebug>
#include "the_player.h"
#include "the_button.h"
#include "profile_page.h"
#include "chat_page.h"
#include "app_settings.h"
#include <QStackedWidget>
#include <QButtonGroup>
#include <QGraphicsDropShadowEffect>
#include <QMenu>
#include <QEvent>
#include <QBoxLayout>
#include <QStackedLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <functional>
#include <QSizePolicy>
#include <QSlider>
#include <QGroupBox>
#include <QTimer>
#include <QTime>
#include <QtGlobal>
#include <algorithm>
#include <limits>
#include <QVector>
#include <QTextToSpeech>
#include <QShortcut>
#include <QKeySequence>
#include <QDialog>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QRegularExpression>
#include <QTextStream>
#include <QSet>

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

struct SubtitleCue {
    qint64 startMs = 0;
    qint64 endMs = 0;
    QString text;
};

static qint64 timeToMs(const QString &timeString) {
    const QStringList parts = timeString.split(QRegularExpression("[,:]"));
    if (parts.size() != 4) {
        return 0;
    }
    const qint64 hours = parts.at(0).toLongLong();
    const qint64 minutes = parts.at(1).toLongLong();
    const qint64 seconds = parts.at(2).toLongLong();
    const qint64 millis = parts.at(3).toLongLong();
    return (((hours * 60 + minutes) * 60) + seconds) * 1000 + millis;
}

static QVector<SubtitleCue> parseSrtFile(const QString &path) {
    QVector<SubtitleCue> cues;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "parseSrtFile: Failed to open file:" << path;
        return cues;
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QString line;
    while (!stream.atEnd()) {
        line = stream.readLine().trimmed();
        if (line.isEmpty()) {
            continue;
        }
        // Skip index line (should be a number)
        bool isNumber = false;
        line.toInt(&isNumber);
        if (!isNumber) {
            qDebug() << "parseSrtFile: Expected index line, got:" << line;
            continue;
        }
        
        // Read timing line
        const QString timingLine = stream.readLine().trimmed();
        const QStringList timeParts = timingLine.split("-->");
        if (timeParts.size() != 2) {
            qDebug() << "parseSrtFile: Invalid timing line:" << timingLine;
            continue;
        }
        SubtitleCue cue;
        cue.startMs = timeToMs(timeParts.at(0).trimmed());
        cue.endMs = timeToMs(timeParts.at(1).trimmed());
        if (cue.startMs == 0 && cue.endMs == 0) {
            qDebug() << "parseSrtFile: Failed to parse timing:" << timingLine;
            continue;
        }

        QStringList textLines;
        while (!stream.atEnd()) {
            const QString textLine = stream.readLine();
            if (textLine.trimmed().isEmpty()) {
                break;
            }
            textLines << textLine.trimmed();
        }
        cue.text = textLines.join(' ');
        if (!cue.text.isEmpty()) {
            cues.push_back(cue);
            qDebug() << "parseSrtFile: Loaded cue:" << cue.startMs << "-" << cue.endMs << ":" << cue.text;
        }
    }
    qDebug() << "parseSrtFile: Total cues loaded:" << cues.size();
    return cues;
}

class SubtitleController : public QObject {
    Q_OBJECT
public:
    explicit SubtitleController(QObject *parent = nullptr)
        : QObject(parent) {}

    void setEnabled(bool enabled) {
        enabled_ = enabled;
        if (!enabled_) {
            emit subtitleChanged(QString());
        }
    }

    void setLanguage(const QString &language) {
        if (language_ == language) {
            return;
        }
        language_ = language;
        if (!currentVideoPath_.isEmpty()) {
            loadForVideo(currentVideoPath_);
        }
    }

    void setActiveVideo(const QString &videoPath) {
        currentVideoPath_ = videoPath;
        loadForVideo(videoPath);
    }

public slots:
    void handlePosition(qint64 position) {
        if (!enabled_ || cues_.isEmpty()) {
            return;
        }
        if (currentCueIndex_ >= 0) {
            const auto &cue = cues_.at(currentCueIndex_);
            if (position >= cue.startMs && position <= cue.endMs) {
                return;
            }
        }
        for (int i = 0; i < cues_.size(); ++i) {
            const auto &cue = cues_.at(i);
            if (position >= cue.startMs && position <= cue.endMs) {
                currentCueIndex_ = i;
                emit subtitleChanged(cue.text);
                return;
            }
        }
        currentCueIndex_ = -1;
        emit subtitleChanged(QString());
    }

signals:
    void subtitleChanged(const QString &text);
    void subtitleAvailabilityChanged(bool available);

private:
    void loadForVideo(const QString &videoPath) {
        cues_.clear();
        currentCueIndex_ = -1;
        bool availableBefore = hasActiveFile_;
        hasActiveFile_ = false;

        if (videoPath.isEmpty()) {
            emit subtitleChanged(QString());
            if (availableBefore != hasActiveFile_) {
                emit subtitleAvailabilityChanged(hasActiveFile_);
            }
            return;
        }

        const QFileInfo info(videoPath);
        const QString baseName = info.completeBaseName();
        const QString dirPath = info.absolutePath();
        QString candidate = QString("%1/%2_%3.srt").arg(dirPath, baseName, language_);
        qDebug() << "SubtitleController: Looking for subtitle file:" << candidate;
        if (!QFile::exists(candidate)) {
            candidate = QString("%1/%2.srt").arg(dirPath, baseName);
            qDebug() << "SubtitleController: Trying fallback:" << candidate;
        }
        if (QFile::exists(candidate)) {
            qDebug() << "SubtitleController: Found subtitle file:" << candidate;
            cues_ = parseSrtFile(candidate);
            hasActiveFile_ = !cues_.isEmpty();
            qDebug() << "SubtitleController: Loaded" << cues_.size() << "subtitle cues";
        } else {
            qDebug() << "SubtitleController: No subtitle file found for" << videoPath;
        }

        if (!hasActiveFile_) {
            emit subtitleChanged(QString());
        }
        if (availableBefore != hasActiveFile_) {
            emit subtitleAvailabilityChanged(hasActiveFile_);
        }
    }

    QVector<SubtitleCue> cues_;
    QString language_ = QStringLiteral("zh");
    QString currentVideoPath_;
    int currentCueIndex_ = -1;
    bool enabled_ = true;
    bool hasActiveFile_ = false;
};

struct HomeCopy {
    QString heroTitle;
    QString heroSubtitleMinutes;
    QString settingsLabel;
    QString dropMeta;
    QString lateBadge;
    QString networkBadge;
    QString selfieFallback;
    QString momentLabelFormat;
    QString captionTemplate;
    QString reactionPromptDefault;
    QString reactionSentFormat;
    QString commentText;
    QString replyButton;
    QString replyDialogTitle;
    QString replyDialogBody;
    QString shareNowButton;
    QString shareDialogTitle;
    QString shareDialogBody;
    QString playLabel;
    QString pauseLabel;
    QString muteLabel;
    QString unmuteLabel;
    QString prevButton;
    QString nextButton;
    QString retakeButton;
    QString retakeHint;
    QString videoStatusLoading;
    QString videoStatusEnded;
    QString videoStatusFailedPrefix;
    QString playbackGenericError;
    QString timeLabelFormat;
    QString locationLabel;
    QString momentFallback;
    QString navHome;
    QString navProfile;
    QString navChat;
    QString dayModeLabel;
    QString nightModeLabel;
    QString subtitleUnavailable;
};

HomeCopy homeCopyFor(AppLanguage language) {
    if (language == AppLanguage::English) {
        return {
            "Today on BeReal Earth",
            "Dual capture delay %1 min · share the moment unfiltered",
            "Settings",
            "2 hours late · Palermo, Buenos Aires",
            "Dual capture delay 2h",
            "4G · 68%",
            "Selfie",
            "Real drop · %1 / %2",
            "“%1 · syncing with friends”",
            "Live reactions from friends",
            "You just sent %1",
            "Luca: this rooftop view reminds me of our midnight test!",
            "Reply",
            "Reply to friend",
            "Get ready to type something real.",
            "Share with friends",
            "Sync completed",
            "Your dual capture has been shared ✅",
            "Pause",
            "Play",
            "Mute",
            "Unmute",
            "Previous moment",
            "Next moment",
            "Retake tip",
            "Tip: BeReal only allows one retake.",
            "Loading...",
            "Playback finished",
            "Playback failed: %1",
            "Unable to play this video",
            "Captured at · %1",
            "Location · Palermo Rooftop",
            "No title",
            "Home",
            "Profile",
            "Chat",
            "Day mode",
            "Night mode",
            "No subtitles for this clip"
        };
    }

    return {
        QString::fromUtf8("今日 · BeReal 地球"),
        QString::fromUtf8("双摄延迟 %1 分钟 · 与朋友共享未修饰瞬间"),
        QString::fromUtf8("设置"),
        QString::fromUtf8("延迟 2 小时 · Palermo, Buenos Aires"),
        QString::fromUtf8("双摄延迟 2 小时"),
        QString::fromUtf8("4G · 68%"),
        QString::fromUtf8("自拍"),
        QString::fromUtf8("真实瞬间 · %1 / %2"),
        QString::fromUtf8("“%1 · 与好友同步”"),
        QString::fromUtf8("好友的实时反应"),
        QString::fromUtf8("你刚刚发送了 %1"),
        QString::fromUtf8("Luca：这景色像极了我们上次的深夜实验！"),
        QString::fromUtf8("回复好友"),
        QString::fromUtf8("回复好友"),
        QString::fromUtf8("准备发一条“真实”评论吧！"),
        QString::fromUtf8("同步到好友"),
        QString::fromUtf8("同步完成"),
        QString::fromUtf8("你的双摄瞬间已经推送给好友 ✅"),
        QString::fromUtf8("暂停"),
        QString::fromUtf8("播放"),
        QString::fromUtf8("静音"),
        QString::fromUtf8("取消静音"),
        QString::fromUtf8("上一瞬间"),
        QString::fromUtf8("下一瞬间"),
        QString::fromUtf8("重拍提示"),
        QString::fromUtf8("贴士：BeReal 只允许一次重拍"),
        QString::fromUtf8("加载中..."),
        QString::fromUtf8("播放结束"),
        QString::fromUtf8("播放失败: %1"),
        QString::fromUtf8("无法播放此视频"),
        QString::fromUtf8("捕捉时间 · %1"),
        QString::fromUtf8("位置 · Palermo Rooftop"),
        QString::fromUtf8("未命名片段"),
        QString::fromUtf8("主页"),
        QString::fromUtf8("资料"),
        QString::fromUtf8("聊天"),
        QString::fromUtf8("日间模式"),
        QString::fromUtf8("夜间模式"),
        QString::fromUtf8("此片段暂无字幕")
    };
}

QStringList discoverSubtitleLanguages(const std::vector<TheButtonInfo> &videos) {
    QSet<QString> languages;
    for (const auto &info : videos) {
        if (!info.url) {
            continue;
        }
        const QFileInfo fileInfo(info.url->toLocalFile());
        const QString dirPath = fileInfo.absolutePath();
        const QString baseName = fileInfo.completeBaseName();
        QDir dir(dirPath);
        const QFileInfoList entries = dir.entryInfoList(QStringList() << QString("%1_*.srt").arg(baseName),
                                                        QDir::Files | QDir::NoSymLinks);
        for (const QFileInfo &entry : entries) {
            const QString entryBase = entry.completeBaseName();
            const int underscore = entryBase.lastIndexOf('_');
            if (underscore == -1) {
                continue;
            }
            const QString lang = entryBase.mid(underscore + 1);
            if (!lang.isEmpty()) {
                languages.insert(lang);
            }
        }
    }
    if (languages.isEmpty()) {
        languages.insert("zh");
        languages.insert("en");
    }
    QStringList result = languages.values();
    std::sort(result.begin(), result.end());
    return result;
}

QLocale localeFor(AppLanguage language) {
    return language == AppLanguage::English ? QLocale(QLocale::English, QLocale::UnitedStates)
                                            : QLocale(QLocale::Chinese, QLocale::China);
}

QString subtitleLanguageDisplayName(const QString &code, AppLanguage interfaceLanguage) {
    const QString upperCode = code.toUpper();
    if (code.compare("zh", Qt::CaseInsensitive) == 0) {
        return interfaceLanguage == AppLanguage::English
                   ? QString("Chinese (%1)").arg(upperCode)
                   : QString::fromUtf8("中文 (%1)").arg(upperCode);
    }
    if (code.compare("en", Qt::CaseInsensitive) == 0) {
        return interfaceLanguage == AppLanguage::English
                   ? QString("English (%1)").arg(upperCode)
                   : QString::fromUtf8("英文 (%1)").arg(upperCode);
    }
    if (interfaceLanguage == AppLanguage::English) {
        return QString("%1 subtitles").arg(upperCode);
    }
    return QString::fromUtf8("%1 字幕").arg(upperCode);
}

class AccessibilitySettingsDialog : public QDialog {
    Q_OBJECT
public:
    AccessibilitySettingsDialog(const AccessibilityPreferences &prefs,
                                const QStringList &subtitleLanguages,
                                AppLanguage language,
                                QWidget *parent = nullptr)
        : QDialog(parent), prefs_(prefs), uiLanguage_(language) {
        setWindowTitle(language == AppLanguage::English
                           ? QStringLiteral("Accessibility & Internationalization")
                           : QString::fromUtf8("无障碍与多语言设置"));
        
        // 应用父窗口的样式表，确保对话框使用正确的主题
        if (parent) {
            setStyleSheet(parent->styleSheet());
        }
        
        auto *layout = new QVBoxLayout(this);
        narrationBox_ = new QCheckBox(language == AppLanguage::English
                                          ? QStringLiteral("Voice narration for low-vision users")
                                          : QString::fromUtf8("视障语音播报"));
        narrationBox_->setChecked(prefs_.narrationEnabled);

        subtitlesBox_ = new QCheckBox(language == AppLanguage::English
                                          ? QStringLiteral("On-screen subtitles for Deaf/HoH")
                                          : QString::fromUtf8("听障字幕"));
        subtitlesBox_->setChecked(prefs_.subtitlesEnabled);

        keyboardNavBox_ = new QCheckBox(language == AppLanguage::English
                                            ? QStringLiteral("Keyboard navigation shortcuts")
                                            : QString::fromUtf8("键盘导航支持"));
        keyboardNavBox_->setChecked(prefs_.keyboardNavigationEnabled);

        colorBlindBox_ = new QCheckBox(language == AppLanguage::English
                                           ? QStringLiteral("High-contrast palette for color-blind users")
                                           : QString::fromUtf8("色盲/色弱高对比配色"));
        colorBlindBox_->setChecked(prefs_.colorBlindPaletteEnabled);

        auto *comboForm = new QFormLayout();
        languageCombo_ = new QComboBox(this);
        languageCombo_->addItem(QString::fromUtf8("中文"), static_cast<int>(AppLanguage::Chinese));
        languageCombo_->addItem(QStringLiteral("English"), static_cast<int>(AppLanguage::English));
        languageCombo_->setCurrentIndex(prefs_.interfaceLanguage == AppLanguage::Chinese ? 0 : 1);

        subtitleCombo_ = new QComboBox(this);
        for (const QString &code : subtitleLanguages) {
            subtitleCombo_->addItem(subtitleLanguageDisplayName(code, language), code);
        }
        const int defaultIndex = subtitleCombo_->findData(prefs_.subtitleLanguage);
        subtitleCombo_->setCurrentIndex(defaultIndex >= 0 ? defaultIndex : 0);

        comboForm->addRow(language == AppLanguage::English
                              ? QStringLiteral("Interface language")
                              : QString::fromUtf8("界面语言"),
                          languageCombo_);
        comboForm->addRow(language == AppLanguage::English
                              ? QStringLiteral("Subtitle language")
                              : QString::fromUtf8("字幕语言"),
                          subtitleCombo_);

        // 视频显示参数调整
        auto *videoParamsGroup = new QGroupBox(language == AppLanguage::English
                                                  ? QStringLiteral("Video Display Parameters")
                                                  : QString::fromUtf8("视频显示参数"),
                                              this);
        auto *videoParamsLayout = new QFormLayout(videoParamsGroup);
        
        brightnessSlider_ = new QSlider(Qt::Horizontal, this);
        brightnessSlider_->setRange(0, 200);  // 0.0-2.0, 步进0.01
        brightnessSlider_->setValue(static_cast<int>(prefs_.brightness * 100));
        brightnessLabel_ = new QLabel(QString::number(prefs_.brightness, 'f', 2), this);
        connect(brightnessSlider_, &QSlider::valueChanged, this, [this](int value) {
            brightnessLabel_->setText(QString::number(value / 100.0, 'f', 2));
        });
        auto *brightnessLayout = new QHBoxLayout();
        brightnessLayout->addWidget(brightnessSlider_);
        brightnessLayout->addWidget(brightnessLabel_);
        videoParamsLayout->addRow(language == AppLanguage::English
                                      ? QStringLiteral("Brightness")
                                      : QString::fromUtf8("亮度"),
                                  brightnessLayout);
        
        contrastSlider_ = new QSlider(Qt::Horizontal, this);
        contrastSlider_->setRange(0, 200);
        contrastSlider_->setValue(static_cast<int>(prefs_.contrast * 100));
        contrastLabel_ = new QLabel(QString::number(prefs_.contrast, 'f', 2), this);
        connect(contrastSlider_, &QSlider::valueChanged, this, [this](int value) {
            contrastLabel_->setText(QString::number(value / 100.0, 'f', 2));
        });
        auto *contrastLayout = new QHBoxLayout();
        contrastLayout->addWidget(contrastSlider_);
        contrastLayout->addWidget(contrastLabel_);
        videoParamsLayout->addRow(language == AppLanguage::English
                                      ? QStringLiteral("Contrast")
                                      : QString::fromUtf8("对比度"),
                                  contrastLayout);
        
        saturationSlider_ = new QSlider(Qt::Horizontal, this);
        saturationSlider_->setRange(0, 200);
        saturationSlider_->setValue(static_cast<int>(prefs_.saturation * 100));
        saturationLabel_ = new QLabel(QString::number(prefs_.saturation, 'f', 2), this);
        connect(saturationSlider_, &QSlider::valueChanged, this, [this](int value) {
            saturationLabel_->setText(QString::number(value / 100.0, 'f', 2));
        });
        auto *saturationLayout = new QHBoxLayout();
        saturationLayout->addWidget(saturationSlider_);
        saturationLayout->addWidget(saturationLabel_);
        videoParamsLayout->addRow(language == AppLanguage::English
                                      ? QStringLiteral("Saturation")
                                      : QString::fromUtf8("饱和度"),
                                  saturationLayout);
        
        playbackRateSlider_ = new QSlider(Qt::Horizontal, this);
        playbackRateSlider_->setRange(25, 400);  // 0.25-4.0, 步进0.01
        playbackRateSlider_->setValue(static_cast<int>(prefs_.playbackRate * 100));
        playbackRateLabel_ = new QLabel(QString::number(prefs_.playbackRate, 'f', 2) + "x", this);
        connect(playbackRateSlider_, &QSlider::valueChanged, this, [this](int value) {
            playbackRateLabel_->setText(QString::number(value / 100.0, 'f', 2) + "x");
        });
        auto *playbackRateLayout = new QHBoxLayout();
        playbackRateLayout->addWidget(playbackRateSlider_);
        playbackRateLayout->addWidget(playbackRateLabel_);
        videoParamsLayout->addRow(language == AppLanguage::English
                                      ? QStringLiteral("Playback Speed")
                                      : QString::fromUtf8("播放倍速"),
                                  playbackRateLayout);

        layout->addWidget(narrationBox_);
        layout->addWidget(subtitlesBox_);
        layout->addWidget(keyboardNavBox_);
        layout->addWidget(colorBlindBox_);
        layout->addLayout(comboForm);
        layout->addWidget(videoParamsGroup);

        auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        layout->addWidget(buttonBox);

        connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
            prefs_.narrationEnabled = narrationBox_->isChecked();
            prefs_.subtitlesEnabled = subtitlesBox_->isChecked();
            prefs_.keyboardNavigationEnabled = keyboardNavBox_->isChecked();
            prefs_.colorBlindPaletteEnabled = colorBlindBox_->isChecked();
            prefs_.interfaceLanguage = static_cast<AppLanguage>(languageCombo_->currentData().toInt());
            prefs_.subtitleLanguage = subtitleCombo_->currentData().toString();
            prefs_.brightness = brightnessSlider_->value() / 100.0;
            prefs_.contrast = contrastSlider_->value() / 100.0;
            prefs_.saturation = saturationSlider_->value() / 100.0;
            prefs_.playbackRate = playbackRateSlider_->value() / 100.0;
            accept();
        });
        connect(buttonBox, &QDialogButtonBox::rejected, this, &AccessibilitySettingsDialog::reject);
    }

    AccessibilityPreferences preferences() const { return prefs_; }

private:
    AccessibilityPreferences prefs_;
    AppLanguage uiLanguage_;
    QCheckBox *narrationBox_ = nullptr;
    QCheckBox *subtitlesBox_ = nullptr;
    QCheckBox *keyboardNavBox_ = nullptr;
    QCheckBox *colorBlindBox_ = nullptr;
    QComboBox *languageCombo_ = nullptr;
    QComboBox *subtitleCombo_ = nullptr;
    QSlider *brightnessSlider_ = nullptr;
    QSlider *contrastSlider_ = nullptr;
    QSlider *saturationSlider_ = nullptr;
    QSlider *playbackRateSlider_ = nullptr;
    QLabel *brightnessLabel_ = nullptr;
    QLabel *contrastLabel_ = nullptr;
    QLabel *saturationLabel_ = nullptr;
    QLabel *playbackRateLabel_ = nullptr;
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
    qDebug() << "Qt version: " << QT_VERSION_STR << endl;

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

    AccessibilityPreferences accessibilityPrefs;
    QStringList subtitleLanguages = discoverSubtitleLanguages(videos);
    QTextToSpeech *speech = nullptr;
    auto narrate = [&](const QString &text) {
        if (!accessibilityPrefs.narrationEnabled || !speech) {
            return;
        }
        const QString trimmed = text.trimmed();
        if (trimmed.isEmpty()) {
            return;
        }
        speech->stop();
        speech->say(trimmed);
    };
    if (!subtitleLanguages.contains(accessibilityPrefs.subtitleLanguage)) {
        accessibilityPrefs.subtitleLanguage = subtitleLanguages.value(0, QStringLiteral("zh"));
    }
    HomeCopy homeCopy = homeCopyFor(accessibilityPrefs.interfaceLanguage);
    const int berealDelayMinutes = 8;

    // the widget that will show the video
    QVideoWidget *videoWidget = new QVideoWidget;
    videoWidget->setObjectName("homeVideo");
    videoWidget->setMinimumHeight(420);

    // the QMediaPlayer which controls the playback
    ThePlayer *player = new ThePlayer;
    player->setVideoOutput(videoWidget);
    player->setContent(nullptr, &videos);
    const int defaultVolume = 60;
    player->setVolume(defaultVolume);
    player->setMuted(false);

    QWidget *homePageContent = new QWidget();
    homePageContent->setObjectName("homePage");
    QVBoxLayout *homeLayout = new QVBoxLayout();
    homeLayout->setContentsMargins(48, 48, 48, 48);
    homeLayout->setSpacing(24);
    homePageContent->setLayout(homeLayout);
    
    QScrollArea *homePageScroll = new QScrollArea();
    homePageScroll->setWidget(homePageContent);
    homePageScroll->setWidgetResizable(true);
    homePageScroll->setFrameShape(QFrame::NoFrame);
    homePageScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    homePageScroll->viewport()->setStyleSheet("background: transparent;");
    
    QWidget *homePage = homePageScroll;

    auto *topBar = new QHBoxLayout();
    topBar->setContentsMargins(0, 0, 0, 0);
    topBar->setSpacing(12);

    auto *titleCol = new QVBoxLayout();
    titleCol->setSpacing(4);
    auto *heroTitle = new QLabel(homeCopy.heroTitle);
    heroTitle->setObjectName("heroTitle");
    auto *heroSubtitle = new QLabel(homeCopy.heroSubtitleMinutes.arg(berealDelayMinutes));
    heroSubtitle->setObjectName("heroSubtitle");
    titleCol->addWidget(heroTitle);
    titleCol->addWidget(heroSubtitle);

    topBar->addLayout(titleCol);
    topBar->addStretch();

    auto *settingsButton = new QPushButton(homeCopy.settingsLabel, homePage);
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
    auto *dropMeta = new QLabel(homeCopy.dropMeta, berealCard);
    dropMeta->setObjectName("dropMeta");
    identityCol->addWidget(displayName);
    identityCol->addWidget(dropMeta);

    auto *momentLabel = new QLabel(videos.empty()
                                       ? homeCopy.momentLabelFormat.arg(0).arg(0)
                                       : homeCopy.momentLabelFormat.arg(1).arg(videos.size()),
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
    captureFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    auto *captureStack = new QStackedLayout(captureFrame);
    captureStack->setStackingMode(QStackedLayout::StackAll);
    captureStack->setContentsMargins(0, 0, 0, 0);

    auto *overlayLayer = new QWidget(captureFrame);
    overlayLayer->setObjectName("captureOverlay");
    overlayLayer->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    overlayLayer->setStyleSheet("background: transparent;");
    overlayLayer->setAutoFillBackground(false);
    auto *overlayLayout = new QVBoxLayout(overlayLayer);
    overlayLayout->setContentsMargins(18, 18, 18, 60);
    overlayLayout->setSpacing(12);

    auto *overlayTop = new QHBoxLayout();
    overlayTop->setSpacing(8);
    auto *lateBadge = new QLabel(homeCopy.lateBadge, overlayLayer);
    lateBadge->setObjectName("lateBadge");
    auto *networkBadge = new QLabel(homeCopy.networkBadge, overlayLayer);
    networkBadge->setObjectName("networkBadge");
    overlayTop->addWidget(lateBadge);
    overlayTop->addStretch();
    overlayTop->addWidget(networkBadge);
    overlayLayout->addLayout(overlayTop);
    overlayLayout->addStretch();

    auto *overlayBottom = new QHBoxLayout();
    overlayBottom->setSpacing(12);
    overlayBottom->addStretch();
    auto *videoStatusLabel = new QLabel(homeCopy.videoStatusLoading, overlayLayer);
    videoStatusLabel->setObjectName("videoStatusLabel");
    videoStatusLabel->setAlignment(Qt::AlignCenter);
    videoStatusLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    videoStatusLabel->hide();
    overlayBottom->addWidget(videoStatusLabel, 0, Qt::AlignBottom);
    overlayBottom->addStretch();
    auto *selfieBubble = new QLabel(homeCopy.selfieFallback, overlayLayer);
    selfieBubble->setObjectName("selfieBubble");
    selfieBubble->setAlignment(Qt::AlignCenter);
    selfieBubble->setScaledContents(true);
    overlayBottom->addWidget(selfieBubble, 0, Qt::AlignBottom);

    auto *reactionRow = new QHBoxLayout();
    reactionRow->setSpacing(8);
    auto *reactionPrompt = new QLabel(homeCopy.reactionPromptDefault, berealCard);
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

    homeLayout->addWidget(berealCard, 0, Qt::AlignTop);
    homeLayout->addStretch();

    overlayLayout->addLayout(overlayBottom);

    captureStack->addWidget(videoWidget);
    captureStack->addWidget(overlayLayer);
    overlayLayer->show();

    cardLayout->addWidget(captureFrame);

    // 创建独立的字幕显示模块（在视频下方）
    auto *subtitleFrame = new QFrame(berealCard);
    subtitleFrame->setObjectName("subtitleFrame");
    subtitleFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    subtitleFrame->setMinimumHeight(60);
    subtitleFrame->setMaximumHeight(80);
    auto *subtitleLayout = new QHBoxLayout(subtitleFrame);  // 改为水平布局
    subtitleLayout->setContentsMargins(12, 12, 12, 12);
    subtitleLayout->setSpacing(0);
    
    auto *subtitleLabel = new QLabel("", subtitleFrame);
    subtitleLabel->setObjectName("subtitleLabel");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setWordWrap(false);  // 禁用自动换行
    subtitleLabel->setVisible(false);
    subtitleLabel->setMinimumHeight(50);
    subtitleLabel->setTextFormat(Qt::PlainText);
    subtitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    
    subtitleLayout->addWidget(subtitleLabel, 0, Qt::AlignCenter);
    
    cardLayout->addWidget(subtitleFrame);

    auto *controlsBar = new QFrame(berealCard);
    controlsBar->setObjectName("controlsBar");
    auto *controlsLayout = new QVBoxLayout(controlsBar);
    controlsLayout->setContentsMargins(24, 12, 24, 12);
    controlsLayout->setSpacing(10);

    auto *progressSlider = new QSlider(Qt::Horizontal, controlsBar);
    progressSlider->setObjectName("progressSlider");
    progressSlider->setRange(0, 0);
    progressSlider->setEnabled(!videos.empty());
    controlsLayout->addWidget(progressSlider);

    auto *controlRow = new QHBoxLayout();
    controlRow->setSpacing(12);

    auto makePillButton = [&](const QString &label, bool checkable = false) {
        auto *btn = new QPushButton(label, controlsBar);
        btn->setObjectName("pillButton");
        btn->setCursor(Qt::PointingHandCursor);
        btn->setCheckable(checkable);
        return btn;
    };

    auto *playPauseButton = makePillButton(homeCopy.pauseLabel);
    auto *muteButton = makePillButton(homeCopy.muteLabel, true);
    auto *prevMomentButton = makePillButton(homeCopy.prevButton);
    auto *nextMomentButton = makePillButton(homeCopy.nextButton);
    auto *retakeButton = makePillButton(homeCopy.retakeButton);

    controlRow->addWidget(playPauseButton);
    controlRow->addWidget(muteButton);
    controlRow->addWidget(prevMomentButton);
    controlRow->addWidget(nextMomentButton);
    controlRow->addStretch();
    controlRow->addWidget(retakeButton);
    controlsLayout->addLayout(controlRow);

    auto *volumeRow = new QHBoxLayout();
    volumeRow->setSpacing(8);
    auto *volumeIcon = new QLabel(QStringLiteral("🔊"), controlsBar);
    volumeIcon->setObjectName("volumeIcon");
    auto *volumeSlider = new QSlider(Qt::Horizontal, controlsBar);
    volumeSlider->setObjectName("volumeSlider");
    volumeSlider->setRange(0, 100);
    volumeSlider->setCursor(Qt::PointingHandCursor);
    volumeSlider->setValue(player->volume());
    volumeRow->addWidget(volumeIcon);
    volumeRow->addWidget(volumeSlider);
    controlsLayout->addLayout(volumeRow);

    cardLayout->addWidget(controlsBar);

    auto *metaFooter = new QFrame(berealCard);
    metaFooter->setObjectName("metaFooter");
    auto *metaLayout = new QHBoxLayout(metaFooter);
    metaLayout->setContentsMargins(0, 0, 0, 0);
    metaLayout->setSpacing(12);
    auto *timeLabel = new QLabel(homeCopy.timeLabelFormat.arg(QStringLiteral("16:42")), metaFooter);
    timeLabel->setObjectName("metaLabel");
    auto *locationLabel = new QLabel(homeCopy.locationLabel, metaFooter);
    locationLabel->setObjectName("metaLabel");
    metaLayout->addWidget(timeLabel);
    metaLayout->addWidget(locationLabel);
    metaLayout->addStretch();
    auto *shareNowButton = new QPushButton(homeCopy.shareNowButton, metaFooter);
    shareNowButton->setObjectName("shareNowButton");
    metaLayout->addWidget(shareNowButton);
    cardLayout->addWidget(metaFooter);

    auto *commentPanel = new QFrame(berealCard);
    commentPanel->setObjectName("commentPanel");
    auto *commentLayout = new QVBoxLayout(commentPanel);
    commentLayout->setContentsMargins(18, 18, 18, 18);
    commentLayout->setSpacing(8);
    auto *captionLabel = new QLabel(homeCopy.captionTemplate.arg(homeCopy.momentFallback), commentPanel);
    captionLabel->setObjectName("captionLabel");
    captionLabel->setWordWrap(true);
    auto *commentLabel = new QLabel(homeCopy.commentText, commentPanel);
    commentLabel->setObjectName("commentLabel");
    commentLabel->setWordWrap(true);
    auto *replyButton = new QPushButton(homeCopy.replyButton, commentPanel);
    replyButton->setObjectName("replyButton");
    replyButton->setCursor(Qt::PointingHandCursor);
    commentLayout->addWidget(captionLabel);
    commentLayout->addWidget(commentLabel);
    commentLayout->addWidget(replyButton, 0, Qt::AlignRight);
    cardLayout->addWidget(commentPanel);

    SubtitleController *subtitleController = new SubtitleController(homePageContent);
    subtitleController->setLanguage(accessibilityPrefs.subtitleLanguage);
    subtitleController->setEnabled(accessibilityPrefs.subtitlesEnabled);
    bool subtitleAvailable = false;

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
            momentLabel->setText(homeCopy.momentLabelFormat.arg(0).arg(0));
            captionLabel->setText(homeCopy.captionTemplate.arg(homeCopy.momentFallback));
            reactionPrompt->setText(homeCopy.reactionPromptDefault);
            narrate(momentLabel->text());
            return;
        }
        const QFileInfo fileInfo(videos.at(index).url->toLocalFile());
        const QString clipName = fileInfo.completeBaseName().isEmpty()
                                     ? homeCopy.momentFallback
                                     : fileInfo.completeBaseName();
        momentLabel->setText(homeCopy.momentLabelFormat.arg(index + 1).arg(videos.size()));
        captionLabel->setText(homeCopy.captionTemplate.arg(clipName));
        timeLabel->setText(homeCopy.timeLabelFormat.arg(QTime::currentTime().toString("hh:mm")));
        locationLabel->setText(homeCopy.locationLabel);
        reactionPrompt->setText(homeCopy.reactionPromptDefault);
        narrate(QString("%1. %2").arg(momentLabel->text(), captionLabel->text()));
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
            selfieBubble->setText(homeCopy.selfieFallback);
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
        setVideoStatus(homeCopy.videoStatusLoading, true);
        updateMomentMeta(currentVideoIndex);
        updateSelfie(currentVideoIndex);
        player->setMedia(*videos.at(currentVideoIndex).url);
        if (subtitleController) {
            subtitleController->setActiveVideo(videos.at(currentVideoIndex).url->toLocalFile());
        }
        // 应用播放倍速设置
        player->setPlaybackRate(accessibilityPrefs.playbackRate);
        player->play();
    };

    QObject::connect(nextMomentButton, &QPushButton::clicked, [&, playVideoAt]() {
        playVideoAt(currentVideoIndex + 1);
    });
    QObject::connect(prevMomentButton, &QPushButton::clicked, [&, playVideoAt]() {
        playVideoAt(currentVideoIndex - 1);
    });

    QObject::connect(retakeButton, &QPushButton::clicked, [setVideoStatus, &homeCopy, narrate]() {
        setVideoStatus(homeCopy.retakeHint, true);
        narrate(homeCopy.retakeHint);
        QTimer::singleShot(2200, [setVideoStatus]() {
            setVideoStatus(QString(), false);
        });
    });

    QObject::connect(shareNowButton, &QPushButton::clicked, [&, &homeCopy, narrate]() {
        QMessageBox::information(berealCard, homeCopy.shareDialogTitle, homeCopy.shareDialogBody);
        narrate(homeCopy.shareDialogBody);
    });

    QObject::connect(replyButton, &QPushButton::clicked, [&, &homeCopy, narrate]() {
        QMessageBox::information(berealCard, homeCopy.replyDialogTitle, homeCopy.replyDialogBody);
        narrate(homeCopy.replyDialogBody);
    });

    QObject::connect(playPauseButton, &QPushButton::clicked, [player, playPauseButton]() {
        if (player->state() == QMediaPlayer::PlayingState) {
            player->pause();
        } else {
            player->play();
        }
    });

    QObject::connect(player, &QMediaPlayer::stateChanged, [playPauseButton, &homeCopy, narrate](QMediaPlayer::State state) {
        if (state == QMediaPlayer::PlayingState) {
            playPauseButton->setText(homeCopy.pauseLabel);
        } else {
            playPauseButton->setText(homeCopy.playLabel);
            narrate(homeCopy.playLabel);
        }
    });

    QObject::connect(muteButton, &QPushButton::toggled, [player, muteButton, &homeCopy](bool checked) {
        player->setMuted(checked);
        muteButton->setText(checked ? homeCopy.unmuteLabel : homeCopy.muteLabel);
    });

    QObject::connect(progressSlider, &QSlider::sliderReleased, [player, progressSlider]() {
        player->setPosition(progressSlider->value());
    });
    QObject::connect(progressSlider, &QSlider::sliderMoved, [player](int value) {
        player->setPosition(value);
    });

    QObject::connect(player, &QMediaPlayer::durationChanged, [progressSlider](qint64 duration) {
        const int sliderMax = duration > 0
                                  ? static_cast<int>(std::min<qint64>(duration, std::numeric_limits<int>::max()))
                                  : 0;
        progressSlider->setMaximum(sliderMax);
        progressSlider->setEnabled(sliderMax > 0);
    });
    QObject::connect(player, &QMediaPlayer::positionChanged, [progressSlider](qint64 position) {
        if (!progressSlider->isSliderDown()) {
            const int sliderPos = static_cast<int>(std::min<qint64>(position, std::numeric_limits<int>::max()));
            progressSlider->setValue(sliderPos);
        }
    });

    QObject::connect(volumeSlider, &QSlider::valueChanged, [player, muteButton](int value) {
        player->setVolume(value);
        if (value == 0 && !muteButton->isChecked()) {
            muteButton->setChecked(true);
        } else if (value > 0 && muteButton->isChecked()) {
            muteButton->setChecked(false);
        }
    });
    QObject::connect(player, &QMediaPlayer::volumeChanged, [volumeSlider](int value) {
        if (volumeSlider->value() != value) {
            volumeSlider->setValue(value);
        }
    });

    QObject::connect(player, &QMediaPlayer::positionChanged, subtitleController, &SubtitleController::handlePosition);
    QObject::connect(subtitleController, &SubtitleController::subtitleChanged,
                     [subtitleLabel, subtitleFrame, &accessibilityPrefs, &subtitleAvailable, &homeCopy](const QString &text) {
        if (!subtitleLabel || !subtitleFrame) {
            return;
        }
        if (!accessibilityPrefs.subtitlesEnabled) {
            subtitleLabel->hide();
            subtitleFrame->hide();
            return;
        }
        if (text.isEmpty()) {
            if (!subtitleAvailable) {
                subtitleLabel->setText(homeCopy.subtitleUnavailable);
                subtitleLabel->show();
                subtitleFrame->show();
            } else {
                subtitleLabel->clear();
                subtitleLabel->hide();
                subtitleFrame->hide();
            }
        } else {
            qDebug() << "SubtitleController: Displaying subtitle:" << text;
            subtitleLabel->setText(text);
            subtitleLabel->show();
            subtitleFrame->show();
            qDebug() << "SubtitleController: Subtitle label visible:" << subtitleLabel->isVisible() 
                     << "text:" << subtitleLabel->text();
        }
    });
    QObject::connect(subtitleController, &SubtitleController::subtitleAvailabilityChanged,
                     [subtitleLabel, subtitleFrame, &accessibilityPrefs, &homeCopy, &subtitleAvailable](bool available) {
        subtitleAvailable = available;
        if (!subtitleLabel || !subtitleFrame) {
            return;
        }
        if (!accessibilityPrefs.subtitlesEnabled) {
            subtitleLabel->hide();
            subtitleFrame->hide();
            return;
        }
        if (!available) {
            subtitleLabel->setText(homeCopy.subtitleUnavailable);
            subtitleLabel->adjustSize();
            subtitleLabel->show();
            subtitleFrame->show();
        } else if (subtitleLabel->text() == homeCopy.subtitleUnavailable) {
            subtitleLabel->clear();
            subtitleLabel->hide();
            subtitleFrame->hide();
        }
    });

    for (auto *btn : reactionButtons) {
        QObject::connect(btn, &QPushButton::clicked, [reactionPrompt, emoji = btn->text(), &homeCopy, narrate]() {
            reactionPrompt->setText(homeCopy.reactionSentFormat.arg(emoji));
            narrate(reactionPrompt->text());
            QTimer::singleShot(2000, [reactionPrompt, &homeCopy]() {
                reactionPrompt->setText(homeCopy.reactionPromptDefault);
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
            videoWidget->show();
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


    QObject::connect(player, &QMediaPlayer::mediaStatusChanged,
                     [setVideoStatus, &homeCopy](QMediaPlayer::MediaStatus status) {
        switch (status) {
        case QMediaPlayer::LoadingMedia:
        case QMediaPlayer::BufferedMedia:
        case QMediaPlayer::StalledMedia:
            setVideoStatus(homeCopy.videoStatusLoading, true);
            break;
        case QMediaPlayer::EndOfMedia:
            setVideoStatus(homeCopy.videoStatusEnded, true);
            break;
        case QMediaPlayer::LoadedMedia:
            setVideoStatus(QString(), false);
            break;
        default:
            break;
        }
    });

    QObject::connect(player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error),
                     [player, setVideoStatus, &homeCopy](QMediaPlayer::Error error) {
        if (error == QMediaPlayer::NoError) {
            return;
        }
        const QString errText = player->errorString().isEmpty()
                                    ? homeCopy.playbackGenericError
                                    : player->errorString();
        setVideoStatus(homeCopy.videoStatusFailedPrefix.arg(errText), true);
    });

    QObject::connect(player, &QMediaPlayer::stateChanged, [setVideoStatus](QMediaPlayer::State state) {
        if (state == QMediaPlayer::PlayingState) {
            setVideoStatus(QString(), false);
        }
    });

    ProfilePage *profilePage = new ProfilePage(videos);
    profilePage->setLanguage(accessibilityPrefs.interfaceLanguage);
    profilePage->setHighContrastMode(accessibilityPrefs.colorBlindPaletteEnabled);
    ChatPage *chatPage = new ChatPage();
    chatPage->setLanguage(accessibilityPrefs.interfaceLanguage);
    chatPage->setHighContrastMode(accessibilityPrefs.colorBlindPaletteEnabled);

    QStackedWidget *stackedPages = new QStackedWidget();
    stackedPages->addWidget(homePage);
    stackedPages->addWidget(profilePage);
    stackedPages->addWidget(chatPage);

    QWidget window;
    window.setObjectName("appRoot");
    QVBoxLayout *top = new QVBoxLayout();
    top->setContentsMargins(28, 24, 28, 32);
    top->setSpacing(16);
    window.setLayout(top);
    window.setWindowTitle("tomeo");
    window.setMinimumSize(420, 720);

    speech = new QTextToSpeech(&window);
    speech->setLocale(localeFor(accessibilityPrefs.interfaceLanguage));
    
    // 智能查找assets目录：先查当前工作目录，再查exe所在目录，最后查源码目录
    QString mapAssetPath;
    QString appDir = QApplication::applicationDirPath();
    QStringList searchPaths = {
        QDir::currentPath() + "/assets/home_map.png",
        appDir + "/assets/home_map.png",
        appDir + "/../assets/home_map.png",
        appDir + "/../../assets/home_map.png"
    };
    
    for (const QString &path : searchPaths) {
        if (QFile::exists(path)) {
            mapAssetPath = path;
            break;
        }
    }
    
    if (mapAssetPath.isEmpty()) {
        qDebug() << "warning: missing home background art. Searched in:";
        for (const QString &path : searchPaths) {
            qDebug() << "  -" << path;
        }
        mapAssetPath = searchPaths.first(); // 使用第一个路径作为默认值
    }
    
    // 将Windows路径转换为Qt样式表可用的格式
    // 尝试使用相对路径，如果不行则使用绝对路径
    QString mapUrl;
    QDir appDirObj(appDir);
    QString relativePath = appDirObj.relativeFilePath(mapAssetPath);
    
    // 如果相对路径合理（不超过父目录太多），使用相对路径
    if (!relativePath.startsWith("..") || relativePath.count("../") < 3) {
        relativePath.replace('\\', '/');
        mapUrl = relativePath;
        qDebug() << "Using relative path:" << mapUrl;
    } else {
        // 否则使用绝对路径，转换为正斜杠格式
        QString normalizedPath = QDir::cleanPath(mapAssetPath);
        mapUrl = normalizedPath;
        mapUrl.replace('\\', '/');
        qDebug() << "Using absolute path:" << mapUrl;
    }
    
    // 验证文件是否存在并输出调试信息
    if (!QFile::exists(mapAssetPath)) {
        qDebug() << "Warning: Background image file not found:" << mapAssetPath;
    } else {
        qDebug() << "Background image file exists:" << mapAssetPath;
        qDebug() << "Style sheet path:" << mapUrl;
    }
    
    auto buildNightStyle = [&](const QString &map) {
        return QString(
            "QWidget#appRoot { background-color: #00040d; }"
            "QWidget#homePage {"
            "  background-color: #01030a;"
            "  background-image: url(%1);"
            "  background-position: center;"
            "  background-repeat: no-repeat;"
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
            "QWidget#captureOverlay { background: transparent; border-radius: 32px; }"
            "QFrame#subtitleFrame { background: transparent; }"
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
            "QLabel#subtitleLabel {"
            "  background-color: rgba(0,0,0,0.9);"
            "  color: #ffffff;"
            "  font-size: 18px;"
            "  font-weight: 700;"
            "  padding: 12px 16px;"
            "  border-radius: 22px;"
            "  border: 2px solid rgba(255,255,255,0.5);"
            "  min-height: 50px;"
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
            "QFrame#controlsBar {"
            "  background-color: rgba(0,0,0,0.65);"
            "  border-radius: 18px;"
            "}"
            "QLabel#volumeIcon { color: #9db6ff; font-size: 14px; }"
            "QSlider#volumeSlider::groove {"
            "  height: 4px;"
            "  background: rgba(255,255,255,0.25);"
            "  border-radius: 2px;"
            "}"
            "QSlider#volumeSlider::handle {"
            "  width: 14px;"
            "  background: rgba(255,255,255,0.9);"
            "  border-radius: 7px;"
            "  margin: -6px 0;"
            "}"
            "QSlider#volumeSlider::sub-page {"
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
            "QDialog { background-color: rgba(2,8,20,0.98); color: #ffffff; }"
            "QDialog * { color: #ffffff; }"
            "QCheckBox { color: #ffffff; }"
            "QCheckBox::indicator { border: 2px solid rgba(157,182,255,0.6); background-color: rgba(2,8,20,0.8); border-radius: 4px; }"
            "QCheckBox::indicator:checked { background-color: rgba(93,155,255,0.9); }"
            "QComboBox { background-color: rgba(15,30,55,0.9); color: #ffffff; border: 1px solid rgba(63,134,255,0.35); border-radius: 8px; padding: 4px 8px; }"
            "QComboBox::drop-down { border: none; }"
            "QComboBox::down-arrow { image: none; border-left: 4px solid transparent; border-right: 4px solid transparent; border-top: 6px solid #ffffff; }"
            "QComboBox QAbstractItemView { background-color: rgba(15,30,55,0.95); color: #ffffff; selection-background-color: rgba(93,155,255,0.7); }"
            "QGroupBox { color: #ffffff; border: 1px solid rgba(63,134,255,0.35); border-radius: 8px; margin-top: 12px; padding-top: 12px; }"
            "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; padding: 0 8px; color: #ffffff; }"
            "QFormLayout QLabel { color: #ffffff; }"
            "QLabel { color: #ffffff; }"
            "QSlider { color: #ffffff; }"
            "QSlider::groove:horizontal { background: rgba(255,255,255,0.2); height: 4px; border-radius: 2px; }"
            "QSlider::handle:horizontal { background: #ffffff; width: 16px; border-radius: 8px; margin: -6px 0; }"
            "QSlider::sub-page:horizontal { background: rgba(93,155,255,0.85); border-radius: 2px; }"
            "QDialogButtonBox QPushButton { background-color: rgba(93,155,255,0.7); color: #ffffff; border: none; border-radius: 6px; padding: 6px 16px; }"
            "QDialogButtonBox QPushButton:hover { background-color: rgba(93,155,255,0.9); }"
        ).arg(map);
    };

    auto buildDayStyle = [&](const QString &map) {
        return QString(
            "QWidget#appRoot { background-color: #f4f7ff; }"
            "QWidget#homePage {"
            "  background-color: #e7edf7;"
            "  background-image: url(%1);"
            "  background-position: center;"
            "  background-repeat: no-repeat;"
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
            "QLabel#subtitleLabel {"
            "  background-color: rgba(12,18,40,0.95);"
            "  color: #ffffff;"
            "  font-size: 18px;"
            "  font-weight: 700;"
            "  padding: 12px 16px;"
            "  border-radius: 22px;"
            "  border: 2px solid rgba(255,255,255,0.5);"
            "  min-height: 50px;"
            "  max-width: none;"
            "}"
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
            "QFrame#controlsBar {"
            "  background-color: rgba(255,255,255,0.92);"
            "  border-radius: 18px;"
            "  border: 1px solid rgba(58,82,132,0.2);"
            "}"
            "QLabel#volumeIcon { color: #20324f; font-size: 14px; }"
            "QSlider#volumeSlider::groove { background: rgba(32,50,90,0.15); height: 4px; border-radius: 2px; }"
            "QSlider#volumeSlider::handle { width: 14px; background: #3353b3; border-radius: 7px; margin: -6px 0; }"
            "QSlider#volumeSlider::sub-page { background: #6f8dff; border-radius: 2px; }"
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
        ).arg(map);
    };

    auto buildColorBlindStyle = [&](const QString &) {
        return QString(
            "QWidget#appRoot { background-color: #000000; }"
            "QWidget#homePage { background-color: #000000; }"
            "QLabel#heroTitle { font-size: 22px; font-weight: 700; color: #f4c430; }"
            "QLabel#heroSubtitle { color: #ffffff; }"
            "QPushButton#settingsButton {"
            "  background-color: #f4c430;"
            "  color: #000;"
            "  border-radius: 22px;"
            "  padding: 10px 22px;"
            "  font-weight: 700;"
            "}"
            "QFrame#berealCard {"
            "  background: #0f0f0f;"
            "  border-radius: 40px;"
            "  border: 2px solid #f4c430;"
            "}"
            "QLabel#momentLabel { color: #f4c430; font-weight: 700; }"
            "QFrame#captureFrame { background: #000; border-radius: 32px; border: 2px solid #f4c430; }"
            "QVideoWidget#homeVideo { border-radius: 32px; }"
            "QLabel#lateBadge { background-color: #f4c430; color: #000; border-radius: 18px; padding: 6px 14px; font-weight: 700; }"
            "QLabel#networkBadge { color: #f4c430; }"
            "QLabel#videoStatusLabel { background-color: #000; color: #f4c430; border: 2px solid #f4c430; border-radius: 24px; padding: 10px 24px; }"
            "QLabel#subtitleLabel { background-color: #000; color: #f4c430; border: 2px solid #f4c430; border-radius: 22px; padding: 12px 16px; font-size: 18px; font-weight: 700; min-height: 50px; max-width: none; }"
            "QPushButton#pillButton { background-color: #f4c430; color: #000; border-radius: 18px; padding: 10px 18px; font-weight: 700; }"
            "QPushButton#reactionButton { background-color: #000; color: #f4c430; border: 2px solid #f4c430; }"
            "QFrame#commentPanel { background: #0f0f0f; border-radius: 26px; border: 2px solid #f4c430; }"
            "QPushButton#replyButton { color: #f4c430; font-weight: 700; }"
            "QFrame#floatingNav { background-color: #000; border: 2px solid #f4c430; border-radius: 28px; }"
            "QPushButton#navButton { color: #f4c430; font-weight: 700; }"
            "QPushButton#navButton:checked { background-color: #f4c430; color: #000; border-radius: 18px; }"
            "QSlider#progressSlider::groove { background: #f4c430; height: 4px; }"
            "QSlider#progressSlider::handle { width: 16px; background: #ffffff; border-radius: 8px; margin: -6px 0; }"
            "QFrame#controlsBar { background-color: rgba(0,0,0,0.9); border: 2px solid #f4c430; border-radius: 18px; }"
            "QLabel#volumeIcon { color: #f4c430; font-size: 14px; }"
            "QSlider#volumeSlider::groove { background: #f4c430; height: 4px; border-radius: 2px; }"
            "QSlider#volumeSlider::handle { width: 14px; background: #ffffff; border-radius: 7px; margin: -6px 0; }"
            "QSlider#volumeSlider::sub-page { background: #ffffff; border-radius: 2px; }"
            "QDialog { background-color: #0f0f0f; color: #f4c430; }"
            "QCheckBox { color: #f4c430; }"
            "QCheckBox::indicator { border: 2px solid #f4c430; background-color: #000000; border-radius: 4px; }"
            "QCheckBox::indicator:checked { background-color: #f4c430; }"
            "QComboBox { background-color: #000000; color: #f4c430; border: 2px solid #f4c430; border-radius: 8px; padding: 4px 8px; }"
            "QComboBox::drop-down { border: none; }"
            "QComboBox::down-arrow { image: none; border-left: 4px solid transparent; border-right: 4px solid transparent; border-top: 6px solid #f4c430; }"
            "QComboBox QAbstractItemView { background-color: #000000; color: #f4c430; selection-background-color: #f4c430; selection-color: #000000; }"
            "QGroupBox { color: #f4c430; border: 2px solid #f4c430; border-radius: 8px; margin-top: 12px; padding-top: 12px; }"
            "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; padding: 0 8px; color: #f4c430; }"
            "QFormLayout QLabel { color: #f4c430; }"
            "QLabel { color: #f4c430; }"
            "QDialog * { color: #f4c430; }"
            "QSlider { color: #f4c430; }"
            "QSlider::groove:horizontal { background: rgba(244,196,48,0.3); height: 4px; border-radius: 2px; }"
            "QSlider::handle:horizontal { background: #f4c430; width: 16px; border-radius: 8px; margin: -6px 0; }"
            "QSlider::sub-page:horizontal { background: #f4c430; border-radius: 2px; }"
            "QDialogButtonBox QPushButton { background-color: #f4c430; color: #000000; border: none; border-radius: 6px; padding: 6px 16px; font-weight: 700; }"
            "QDialogButtonBox QPushButton:hover { background-color: #ffd700; }"
        );
    };

    enum class ThemePreset { Night, Day };
    ThemePreset currentThemePreset = ThemePreset::Night;
    auto applyTheme = [&](ThemePreset preset) {
        currentThemePreset = preset;
        const bool highContrast = accessibilityPrefs.colorBlindPaletteEnabled;
        if (highContrast) {
            window.setStyleSheet(buildColorBlindStyle(mapUrl));
        } else if (preset == ThemePreset::Night) {
            window.setStyleSheet(buildNightStyle(mapUrl));
        } else {
            window.setStyleSheet(buildDayStyle(mapUrl));
        }
        if (profilePage) {
            profilePage->setHighContrastMode(highContrast);
        }
        if (chatPage) {
            chatPage->setHighContrastMode(highContrast);
        }
    };
    applyTheme(ThemePreset::Night);

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
    QPushButton *chatNavButton = nullptr;

    struct NavSpec {
        QString label;
        int index;
        QPushButton **slot;
    };
    const std::vector<NavSpec> navSpecs = {
        {homeCopy.navHome, 0, &homeNavButton},
        {homeCopy.navProfile, 1, &profileNavButton},
        {homeCopy.navChat, 2, &chatNavButton}
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
        QObject::connect(button, &QPushButton::toggled, [stackedPages, targetIndex, button, narrate](bool checked) {
            if (checked) {
                stackedPages->setCurrentIndex(targetIndex);
                narrate(button->text());
            }
        });

        if (spec.slot) {
            *(spec.slot) = button;
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

    QVector<QShortcut *> keyboardShortcuts;
    auto rebuildKeyboardShortcuts = [&]() {
        for (auto *shortcut : keyboardShortcuts) {
            if (shortcut) {
                shortcut->deleteLater();
            }
        }
        keyboardShortcuts.clear();
        if (!accessibilityPrefs.keyboardNavigationEnabled) {
            return;
        }
        auto addShortcut = [&](const QKeySequence &sequence, const std::function<void()> &handler) {
            auto *shortcut = new QShortcut(sequence, &window);
            keyboardShortcuts.push_back(shortcut);
            QObject::connect(shortcut, &QShortcut::activated, &window, handler);
        };
        if (homeNavButton) {
            addShortcut(QKeySequence(Qt::ALT | Qt::Key_1), [homeNavButton]() {
                homeNavButton->setChecked(true);
            });
        }
        if (profileNavButton) {
            addShortcut(QKeySequence(Qt::ALT | Qt::Key_2), [profileNavButton]() {
                profileNavButton->setChecked(true);
            });
        }
        if (chatNavButton) {
            addShortcut(QKeySequence(Qt::ALT | Qt::Key_3), [chatNavButton]() {
                chatNavButton->setChecked(true);
            });
        }
        addShortcut(QKeySequence(Qt::Key_Space), [playPauseButton]() {
            playPauseButton->click();
        });
        addShortcut(QKeySequence(Qt::Key_M), [muteButton]() {
            muteButton->click();
        });
        addShortcut(QKeySequence(Qt::ALT | Qt::Key_Left), [prevMomentButton]() {
            prevMomentButton->click();
        });
        addShortcut(QKeySequence(Qt::ALT | Qt::Key_Right), [nextMomentButton]() {
            nextMomentButton->click();
        });
        addShortcut(QKeySequence(Qt::ALT | Qt::Key_R), [retakeButton]() {
            retakeButton->click();
        });
        addShortcut(QKeySequence(Qt::Key_Right), [player]() {
            const qint64 duration = std::max<qint64>(0, player->duration());
            const qint64 nextPos = player->position() + 5000;
            player->setPosition(std::min(nextPos, duration));
        });
        addShortcut(QKeySequence(Qt::Key_Left), [player]() {
            const qint64 nextPos = player->position() - 5000;
            player->setPosition(std::max<qint64>(0, nextPos));
        });
        addShortcut(QKeySequence(Qt::Key_Up), [volumeSlider]() {
            const int nextValue = qMin(volumeSlider->maximum(), volumeSlider->value() + 5);
            volumeSlider->setValue(nextValue);
        });
        addShortcut(QKeySequence(Qt::Key_Down), [volumeSlider]() {
            const int nextValue = qMax(volumeSlider->minimum(), volumeSlider->value() - 5);
            volumeSlider->setValue(nextValue);
        });
    };

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

    QMenu *settingsMenu = nullptr;
    QAction *dayModeAction = nullptr;
    QAction *nightModeAction = nullptr;
    QAction *accessibilityAction = nullptr;
    if (settingsButton) {
        settingsMenu = new QMenu(settingsButton);
        accessibilityAction = settingsMenu->addAction(accessibilityPrefs.interfaceLanguage == AppLanguage::English
                                                          ? QStringLiteral("Accessibility Center")
                                                          : QString::fromUtf8("无障碍中心"));
        settingsMenu->addSeparator();
        dayModeAction = settingsMenu->addAction(homeCopy.dayModeLabel);
        nightModeAction = settingsMenu->addAction(homeCopy.nightModeLabel);

        QObject::connect(settingsButton, &QPushButton::clicked, [settingsMenu, settingsButton]() {
            settingsMenu->exec(settingsButton->mapToGlobal(QPoint(settingsButton->width(), settingsButton->height())));
        });
        QObject::connect(dayModeAction, &QAction::triggered, [&, applyTheme]() {
            applyTheme(ThemePreset::Day);
        });
        QObject::connect(nightModeAction, &QAction::triggered, [&, applyTheme]() {
            applyTheme(ThemePreset::Night);
        });
    }

    auto applyLocalization = [&]() {
        homeCopy = homeCopyFor(accessibilityPrefs.interfaceLanguage);
        heroTitle->setText(homeCopy.heroTitle);
        heroSubtitle->setText(homeCopy.heroSubtitleMinutes.arg(berealDelayMinutes));
        settingsButton->setText(homeCopy.settingsLabel);
        dropMeta->setText(homeCopy.dropMeta);
        lateBadge->setText(homeCopy.lateBadge);
        networkBadge->setText(homeCopy.networkBadge);
        shareNowButton->setText(homeCopy.shareNowButton);
        replyButton->setText(homeCopy.replyButton);
        commentLabel->setText(homeCopy.commentText);
        if (selfieBubble->pixmap() && !selfieBubble->pixmap()->isNull()) {
            selfieBubble->setText("");
        } else {
            selfieBubble->setText(homeCopy.selfieFallback);
        }
        prevMomentButton->setText(homeCopy.prevButton);
        nextMomentButton->setText(homeCopy.nextButton);
        retakeButton->setText(homeCopy.retakeButton);
        playPauseButton->setText(player->state() == QMediaPlayer::PlayingState ? homeCopy.pauseLabel : homeCopy.playLabel);
        muteButton->setText(muteButton->isChecked() ? homeCopy.unmuteLabel : homeCopy.muteLabel);
        timeLabel->setText(homeCopy.timeLabelFormat.arg(QTime::currentTime().toString("hh:mm")));
        locationLabel->setText(homeCopy.locationLabel);
        captionLabel->setText(homeCopy.captionTemplate.arg(homeCopy.momentFallback));
        reactionPrompt->setText(homeCopy.reactionPromptDefault);
        if (homeNavButton) {
            homeNavButton->setText(homeCopy.navHome);
        }
        if (profileNavButton) {
            profileNavButton->setText(homeCopy.navProfile);
        }
        if (chatNavButton) {
            chatNavButton->setText(homeCopy.navChat);
        }
        if (dayModeAction) {
            dayModeAction->setText(homeCopy.dayModeLabel);
        }
        if (nightModeAction) {
            nightModeAction->setText(homeCopy.nightModeLabel);
        }
        if (accessibilityAction) {
            accessibilityAction->setText(accessibilityPrefs.interfaceLanguage == AppLanguage::English
                                             ? QStringLiteral("Accessibility Center")
                                             : QString::fromUtf8("无障碍中心"));
        }
        if (profilePage) {
            profilePage->setLanguage(accessibilityPrefs.interfaceLanguage);
        }
        if (chatPage) {
            chatPage->setLanguage(accessibilityPrefs.interfaceLanguage);
        }
        if (speech) {
            speech->setLocale(localeFor(accessibilityPrefs.interfaceLanguage));
        }
        if (videos.empty()) {
            updateMomentMeta(0);
        } else {
            updateMomentMeta(currentVideoIndex);
        }
        if (!subtitleAvailable && accessibilityPrefs.subtitlesEnabled && subtitleLabel) {
            subtitleLabel->setText(homeCopy.subtitleUnavailable);
            subtitleLabel->setVisible(true);
        }
    };

    auto applyAccessibilityBehavior = [&]() {
        subtitleController->setLanguage(accessibilityPrefs.subtitleLanguage);
        subtitleController->setEnabled(accessibilityPrefs.subtitlesEnabled);
        if (!accessibilityPrefs.subtitlesEnabled && subtitleLabel) {
            subtitleLabel->hide();
        } else if (!subtitleAvailable && subtitleLabel) {
            subtitleLabel->setText(homeCopy.subtitleUnavailable);
            subtitleLabel->setVisible(true);
        }
        // 应用播放倍速设置
        if (player) {
            player->setPlaybackRate(accessibilityPrefs.playbackRate);
        }
        rebuildKeyboardShortcuts();
        applyTheme(currentThemePreset);
        if (!accessibilityPrefs.narrationEnabled && speech) {
            speech->stop();
        }
        if (!videos.empty()) {
            subtitleController->setActiveVideo(videos.at(currentVideoIndex).url->toLocalFile());
        }
    };

    applyLocalization();
    applyAccessibilityBehavior();
    playVideoAt(0);

    if (accessibilityAction) {
        QObject::connect(accessibilityAction, &QAction::triggered, [&]() {
            AccessibilitySettingsDialog dialog(accessibilityPrefs, subtitleLanguages,
                                               accessibilityPrefs.interfaceLanguage, &window);
            if (dialog.exec() != QDialog::Accepted) {
                return;
            }
            accessibilityPrefs = dialog.preferences();
            applyLocalization();
            applyAccessibilityBehavior();
            narrate(settingsButton->text());
        });
    }

    // showtime!
    window.show();

    // wait for the app to terminate
    return app.exec();
}

#include "tomeo.moc"




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
#include <QCoreApplication>
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
#include <QPixmap>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QAbstractSlider>
#include <QCheckBox>
#include <QActionGroup>
#include <QStringLiteral>
#include <QStringList>
#include <QFont>
#include <QConicalGradient>
#include <QPainterPath>
#include <memory>
#include "app_settings.h"
#include "shared/language_manager.h"
#include "../capture/popuppanel.h"
#include "shared/narration_manager.h"
#include <QTextToSpeech>
#include <QShortcut>
#include <QKeySequence>
#include <QDialog>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QFormLayout>
#include <QRegularExpression>
#include <QTextStream>
#include <QSet>
#include <QGroupBox>
#include <QVector>
#include <QLocale>
#include <QRandomGenerator>

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
} // namespace

// Colored avatar ring (Instagram-style), use project color scheme for gradient (for HomePage)
class HomePageAvatarRingWidget : public QWidget {
public:
    explicit HomePageAvatarRingWidget(const QIcon *icon, QWidget *parent = nullptr)
        : QWidget(parent) {
        if (icon) {
            icon_.reset(new QIcon(*icon));
        }
        setFixedSize(60, 60);
        setAttribute(Qt::WA_TranslucentBackground, true);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QRectF outerRect = rect().adjusted(1, 1, -1, -1);

        // Outer ring: multi-color gradient ring
        QConicalGradient grad(outerRect.center(), 0);
        // Multi-segment gradient for smoother color transitions, minimize pure black
        grad.setColorAt(0.00, QColor("#FF4F70"));   // Pink
        grad.setColorAt(0.18, QColor("#FF8AA0"));   // Pink → brighter
        grad.setColorAt(0.32, QColor("#6CADFF"));   // Light blue
        grad.setColorAt(0.46, QColor("#3A7DFF"));   // Dark blue
        grad.setColorAt(0.60, QColor("#BFBFBF"));   // Silver gray
        grad.setColorAt(0.74, QColor("#6CADFF"));   // Back to light blue
        grad.setColorAt(0.88, QColor("#FF8AA0"));   // Pink transition again
        grad.setColorAt(1.00, QColor("#FF4F70"));   // Close loop pink

        painter.setPen(Qt::NoPen);
        painter.setBrush(grad);
        painter.drawEllipse(outerRect);

        // Inner ring: dark background + avatar
        const int ringWidth = 3;
        QRectF innerRect = outerRect.adjusted(ringWidth, ringWidth, -ringWidth, -ringWidth);
        // Change inner ring background to dark blue instead of pure black
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
        if (!enabled_) {
            return;
        }
        if (cues_.isEmpty()) {
            return;
        }
        // Check if current subtitle is still within valid range
        if (currentCueIndex_ >= 0 && currentCueIndex_ < cues_.size()) {
            const auto &cue = cues_.at(currentCueIndex_);
            if (position >= cue.startMs && position <= cue.endMs) {
                return; // Still within current subtitle time range, no need to update
            }
        }
        // Find new subtitle
        for (int i = 0; i < cues_.size(); ++i) {
            const auto &cue = cues_.at(i);
            if (position >= cue.startMs && position <= cue.endMs) {
                if (currentCueIndex_ != i) {
                currentCueIndex_ = i;
                    qDebug() << "SubtitleController: Showing subtitle at" << position << "ms:" << cue.text;
                emit subtitleChanged(cue.text);
                }
                return;
            }
        }
        // No matching subtitle found, clear display
        if (currentCueIndex_ >= 0) {
        currentCueIndex_ = -1;
        emit subtitleChanged(QString());
        }
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
        const QString dirPath = QDir::cleanPath(info.absolutePath());
        
        // Build multiple possible subtitle file paths (sorted by priority)
        QStringList searchPaths;
        
        // 1. Same directory as video file, with language suffix: {baseName}_{language}.srt
        searchPaths << QDir::cleanPath(QString("%1/%2_%3.srt").arg(dirPath, baseName, language_));
        
        // 2. Same directory as video file, without language suffix: {baseName}.srt
        searchPaths << QDir::cleanPath(QString("%1/%2.srt").arg(dirPath, baseName));
        
        // 3. Look for subtitles in videos folder under project root (videos directory at same level as src1, src2)
        // This videos directory will be committed, so SRT files should be placed here
        QDir videoDir(dirPath);
        // Try to find project root directory (search upward until finding directory containing videos)
        QDir currentDir = videoDir;
        QString videosPath;
        
        for (int i = 0; i < 5 && currentDir.cdUp(); ++i) {
            QString rootPath = currentDir.absolutePath();
            
            // Check videos directory under root directory (videos folder at same level as src1, src2)
            QString testVideosPath = QDir::cleanPath(QString("%1/videos").arg(rootPath));
            if (QDir(testVideosPath).exists() && videosPath.isEmpty()) {
                videosPath = testVideosPath;
                break; // Stop when found
            }
            }
            
        // Look for subtitle files in videos folder under root directory
        if (!videosPath.isEmpty()) {
                searchPaths << QDir::cleanPath(QString("%1/%2_%3.srt").arg(videosPath, baseName, language_));
                searchPaths << QDir::cleanPath(QString("%1/%2.srt").arg(videosPath, baseName));
            }
        
        // 4. Additional check: if video file path contains videos, also search in videos directory at same level
        if (dirPath.contains("videos", Qt::CaseInsensitive)) {
            // If video is already in videos directory, already searched in steps 1-2
            // Can add other possible videos directory paths here
        }
        
        QString foundPath;
        for (const QString &candidate : searchPaths) {
            qDebug() << "SubtitleController: Looking for subtitle file:" << candidate;
            if (QFile::exists(candidate)) {
                foundPath = candidate;
                qDebug() << "SubtitleController: Found subtitle file:" << foundPath;
                break;
            }
        }
        
        if (!foundPath.isEmpty()) {
            cues_ = parseSrtFile(foundPath);
            hasActiveFile_ = !cues_.isEmpty();
            qDebug() << "SubtitleController: Loaded" << cues_.size() << "subtitle cues from" << foundPath;
            if (cues_.isEmpty()) {
                qDebug() << "SubtitleController: WARNING - Subtitle file found but no cues were parsed. Check file format.";
            }
        } else {
            qDebug() << "SubtitleController: No subtitle file found for video:" << videoPath;
            qDebug() << "SubtitleController: Base name:" << baseName;
            qDebug() << "SubtitleController: Language:" << language_;
            qDebug() << "SubtitleController: Searched paths:";
            for (const QString &path : searchPaths) {
                qDebug() << "  -" << path;
            }
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

QStringList discoverSubtitleLanguages(const std::vector<TheButtonInfo> &videos) {
    QSet<QString> languages;
    for (const auto &info : videos) {
        if (!info.url) {
            continue;
        }
        const QFileInfo fileInfo(info.url->toLocalFile());
        const QString dirPath = fileInfo.absolutePath();
        const QString baseName = fileInfo.completeBaseName();
        
        // Search in same directory as video file
        QDir dir(dirPath);
        QFileInfoList entries = dir.entryInfoList(QStringList() << QString("%1_*.srt").arg(baseName),
                                                        QDir::Files | QDir::NoSymLinks);
        
        // Also search in videos folder under root directory (videos directory at same level as src1, src2)
        QDir currentDir = dir;
        for (int i = 0; i < 5 && currentDir.cdUp(); ++i) {
            QString rootPath = currentDir.absolutePath();
            QString videosPath = QDir::cleanPath(QString("%1/videos").arg(rootPath));
            if (QDir(videosPath).exists()) {
                QDir videosDir(videosPath);
                QFileInfoList videosEntries = videosDir.entryInfoList(QStringList() << QString("%1_*.srt").arg(baseName),
                                                                    QDir::Files | QDir::NoSymLinks);
                entries.append(videosEntries);
                break; // Stop when found
            }
        }
        
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

// Get user avatar path (same function as in FriendItem)
static QString getAvatarPathForUser(const QString &username)
{
    // Map the first five demo users to avatar files 1-5.jpg
    QStringList avatarUsers = {"Alice", "Bob", "Ethan", "Luna", "Olivia"};
    int index = avatarUsers.indexOf(username);
    
    if (index >= 0 && index < 5) {
        QString fileName = QString::number(index + 1) + ".jpg";
        QStringList searchPaths = {
            QDir::currentPath() + "/friends/avatar/" + fileName,
            QDir::currentPath() + "/../friends/avatar/" + fileName,
            QDir::currentPath() + "/src/friends/avatar/" + fileName,
            QDir::currentPath() + "/../src/friends/avatar/" + fileName,
            QDir::currentPath() + "/../../src/friends/avatar/" + fileName,
            QApplication::applicationDirPath() + "/friends/avatar/" + fileName,
            QApplication::applicationDirPath() + "/../friends/avatar/" + fileName,
            QApplication::applicationDirPath() + "/../../friends/avatar/" + fileName,
            QApplication::applicationDirPath() + "/../../src/friends/avatar/" + fileName,
            QApplication::applicationDirPath() + "/../../../src/friends/avatar/" + fileName,
            "friends/avatar/" + fileName,
            "../friends/avatar/" + fileName,
            "src/friends/avatar/" + fileName
        };
        
        for (const QString &path : searchPaths) {
            QString normalizedPath = QDir::cleanPath(path);
            if (QFile::exists(normalizedPath)) {
                return normalizedPath;
            }
        }
    }
    
    return "";
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
                                QMediaPlayer *player,
                                QWidget *parent = nullptr)
        : QDialog(parent), prefs_(prefs), uiLanguage_(language), player_(player) {
        setWindowTitle(language == AppLanguage::English
                           ? QStringLiteral("Accessibility & Internationalization")
                           : QString::fromUtf8("无障碍与多语言设置"));
        
        // Apply parent window's stylesheet to ensure dialog uses correct theme
        if (parent) {
            setStyleSheet(parent->styleSheet());
        }
        
        auto *layout = new QVBoxLayout(this);
        narrationBox_ = new QCheckBox(language == AppLanguage::English
                                          ? QStringLiteral("Screen narration support")
                                          : QString::fromUtf8("屏幕朗读支持"));
        narrationBox_->setChecked(prefs_.narrationEnabled);

        subtitlesBox_ = new QCheckBox(language == AppLanguage::English
                                          ? QStringLiteral("On-screen subtitles")
                                          : QString::fromUtf8("屏幕字幕"));
        subtitlesBox_->setChecked(prefs_.subtitlesEnabled);

        keyboardNavBox_ = new QCheckBox(language == AppLanguage::English
                                            ? QStringLiteral("Keyboard navigation")
                                            : QString::fromUtf8("键盘导航"));
        keyboardNavBox_->setChecked(prefs_.keyboardNavigationEnabled);

        colorBlindBox_ = new QCheckBox(language == AppLanguage::English
                                           ? QStringLiteral("High-contrast mode")
                                           : QString::fromUtf8("高对比度模式"));
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

        // Video display parameter adjustment
        auto *videoParamsGroup = new QGroupBox(language == AppLanguage::English
                                                  ? QStringLiteral("Video Display Parameters")
                                                  : QString::fromUtf8("视频显示参数"),
                                              this);
        auto *videoParamsLayout = new QFormLayout(videoParamsGroup);
        
        brightnessSlider_ = new QSlider(Qt::Horizontal, this);
        brightnessSlider_->setRange(0, 200);  // 0.0-2.0, step 0.01
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
        
        // Only allow playback speeds stably supported by backend: 0.5x / 1.0x / 1.5x / 2.0x
        static const double kRates[] = {0.5, 1.0, 1.5, 2.0};

        auto nearestRate = [](double value) {
            const double kRatesLocal[] = {0.5, 1.0, 1.5, 2.0};
            double best = kRatesLocal[0];
            double bestDiff = std::fabs(value - best);
            for (double r : kRatesLocal) {
                const double d = std::fabs(value - r);
                if (d < bestDiff) {
                    best = r;
                    bestDiff = d;
                }
            }
            return best;
        };

        // Align current preference value to nearest valid playback speed
        prefs_.playbackRate = nearestRate(prefs_.playbackRate);
        
        playbackRateSlider_ = new QSlider(Qt::Horizontal, this);
        // Integer percentage between 0.5–2.0, but we'll automatically "snap" to valid positions in valueChanged
        playbackRateSlider_->setRange(50, 200);
        playbackRateSlider_->setValue(static_cast<int>(prefs_.playbackRate * 100));
        playbackRateLabel_ = new QLabel(QString::number(prefs_.playbackRate, 'f', 2) + "x", this);
        connect(playbackRateSlider_, &QSlider::valueChanged, this, [this, nearestRate](int value) {
            double raw = value / 100.0;
            double snapped = nearestRate(raw);
            int snappedInt = static_cast<int>(snapped * 100 + 0.5);

            // If user drags to non-exact position, automatically snap to nearest position and update slider
            if (snappedInt != value) {
                playbackRateSlider_->blockSignals(true);
                playbackRateSlider_->setValue(snappedInt);
                playbackRateSlider_->blockSignals(false);
            }

            playbackRateLabel_->setText(QString::number(snapped, 'f', 2) + "x");
            prefs_.playbackRate = snapped;

            // Update player playback speed in real-time when dialog is open, let user feel changes immediately
            if (player_) {
                player_->setPlaybackRate(snapped);
            }

            updatePlaybackWarningLabel();
        });
        auto *playbackRateLayout = new QHBoxLayout();
        playbackRateLayout->addWidget(playbackRateSlider_);
        playbackRateLayout->addWidget(playbackRateLabel_);
        videoParamsLayout->addRow(language == AppLanguage::English
                                      ? QStringLiteral("Playback Speed")
                                      : QString::fromUtf8("播放倍速"),
                                  playbackRateLayout);

        // Environment limitation notice: current backend doesn't support real playback speed, always 1.0x
        playbackRateWarning_ = new QLabel(this);
        playbackRateWarning_->setWordWrap(true);
        playbackRateWarning_->setStyleSheet("color: #ff4f70; font-size: 11px;");
        videoParamsLayout->addRow(QString(), playbackRateWarning_);

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
            // playbackRate has already been snapped by nearestRate in valueChanged, use prefs_ directly here
            accept();
        });
        connect(buttonBox, &QDialogButtonBox::rejected, this, &AccessibilitySettingsDialog::reject);

        // When interface language changes, update warning language
        connect(languageCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &AccessibilitySettingsDialog::updatePlaybackWarningLabel);

        // Initialize warning based on current playback speed
        updatePlaybackWarningLabel();
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
    QLabel *playbackRateWarning_ = nullptr;
    QMediaPlayer *player_ = nullptr;

    void updatePlaybackWarningLabel();
};

void AccessibilitySettingsDialog::updatePlaybackWarningLabel()
{
    if (!playbackRateWarning_) {
        return;
    }

    // If current playback speed is 1.0x, don't show warning
    if (qFuzzyCompare(prefs_.playbackRate, 1.0)) {
        playbackRateWarning_->hide();
        return;
    }

    // Select warning text based on interface language
    AppLanguage lang = uiLanguage_;
    if (languageCombo_) {
        const QVariant data = languageCombo_->currentData();
        if (data.isValid()) {
            lang = static_cast<AppLanguage>(data.toInt());
        }
    }

    const QString zh = QString::fromUtf8(
        "当前环境下系统后端不支持音频变速，因此实测只能固定 1.0x，这是平台限制，不是逻辑错误。");
    const QString en = QStringLiteral(
        "On this system the multimedia backend does not support changing the audio playback speed, "
        "so it effectively stays fixed at 1.0x. This is a platform limitation, not a bug in the app.");

    playbackRateWarning_->setText(
        lang == AppLanguage::Chinese ? zh : en);
    playbackRateWarning_->show();
}

// Custom HomePage widget that paints the background image
class HomePageWidget : public QWidget {
public:
    explicit HomePageWidget(QWidget *parent = nullptr)
        : QWidget(parent), gradientAngle_(0.0) {
        setObjectName("homePage");
    }
    
    void setGradientAngle(qreal angle) {
        if (gradientAngle_ != angle) {
            gradientAngle_ = angle;
            update(); // Trigger redraw
        }
    }
    
    void setBackgroundImage(const QString &bgImagePath) {
        if (backgroundPath == bgImagePath) {
            return;
        }
        backgroundPath = bgImagePath;
        backgroundPixmap = QPixmap();
        scaledBackground = QPixmap();
        lastScaledSize = QSize();

        if (backgroundPath.isEmpty()) {
            update();
            return;
        }

        QImageReader reader(backgroundPath);
        reader.setAutoTransform(true);

        const QSize originalSize = reader.size();
        if (originalSize.isValid()) {
            const int maxDimension = 2560;
            const int largestSide = qMax(originalSize.width(), originalSize.height());
            if (largestSide > maxDimension) {
                const double ratio = static_cast<double>(maxDimension) / largestSide;
                reader.setScaledSize(originalSize * ratio);
            }
        }

        const QImage image = reader.read();
        if (!image.isNull()) {
            backgroundPixmap = QPixmap::fromImage(image);
        } else {
            qWarning() << "Failed to load background image:" << backgroundPath << reader.errorString();
        }

        updateScaledBackground(size());
        update(); // 触发重绘
    }

protected:
    void resizeEvent(QResizeEvent *event) override {
        QWidget::resizeEvent(event);
        updateScaledBackground(event->size());
    }

    void paintEvent(QPaintEvent *event) override {
        QWidget::paintEvent(event);
        
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        
        // Draw background image first
        if (!scaledBackground.isNull()) {
            painter.setRenderHint(QPainter::SmoothPixmapTransform);
            
            // Calculate drawing position, only show middle-lower part of image
            // Top of image is cropped, bottom aligned to widget bottom
            const QSize widgetSize = size();
            const QSize pixmapSize = scaledBackground.size();
            int x = (widgetSize.width() - pixmapSize.width()) / 2;
            int y = widgetSize.height() - pixmapSize.height();  // Bottom aligned
            
            // If image height is less than widget height, center it
            if (pixmapSize.height() < widgetSize.height()) {
                y = (widgetSize.height() - pixmapSize.height()) / 2;
            }
            
            // Draw at fixed position, doesn't change with window scrolling
            painter.drawPixmap(x, y, pixmapSize.width(), pixmapSize.height(), scaledBackground);
        }
        
        // Draw colored border (similar to Profile page)
        painter.setRenderHint(QPainter::Antialiasing);
        
        // Outer rectangle slightly inset to avoid clipping
        QRectF outerRect = rect().adjusted(3.0, 3.0, -3.0, -3.0);
        
        // Use multi-color gradient similar to avatar to create a "color ring border"
        // Gradient angle changes with scroll position
        QConicalGradient grad(outerRect.center(), gradientAngle_);
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

private:
    void updateScaledBackground(const QSize &targetSize) {
        if (backgroundPixmap.isNull() || targetSize.isEmpty()) {
            scaledBackground = QPixmap();
            lastScaledSize = QSize();
            return;
        }
        if (lastScaledSize == targetSize) {
            return;
        }
        scaledBackground = backgroundPixmap.scaled(targetSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        lastScaledSize = targetSize;
    }

    QString backgroundPath;
    QPixmap backgroundPixmap;
    QPixmap scaledBackground;
    QSize lastScaledSize;
    qreal gradientAngle_; // Gradient angle, changes with scrolling
};

// read in videos and thumbnails from this directory
// If same letter has multiple formats (.wmv / .mp4 / .mov), only keep **one**, avoid duplicate appearance on homepage and friend feed
std::vector<TheButtonInfo> getInfoIn(std::string loc) {
    std::vector<TheButtonInfo> out;

    QDir dir(QString::fromStdString(loc));
    if (!dir.exists()) {
        qWarning() << "Video directory does not exist:" << dir.absolutePath();
        return out;
    }

    // Collect all candidate video files, group by "same name different extension"
    struct Candidate {
        QFileInfo fi;
        int priority = 0;
    };
    QMap<QString, Candidate> bestByBaseName;

    QDirIterator it(dir);
    while (it.hasNext()) {
        const QString f = it.next();
        QFileInfo fi(f);
        if (!fi.isFile()) {
            continue;
        }

        const QString lower = fi.fileName().toLower();
        QString ext = fi.suffix().toLower();       // Without dot, e.g. "mp4"
        QString base = fi.completeBaseName();      // e.g. "a", "b", "h"

        int pri = 0;
#if defined(_WIN32)
        // On Windows, prefer .wmv, then .mp4, finally .mov
        if (ext == "wmv") pri = 3;
        else if (ext == "mp4") pri = 2;
        else if (ext == "mov") pri = 1;
#else
        // Other platforms prefer mp4 / mov
        if (ext == "mp4") pri = 3;
        else if (ext == "mov") pri = 2;
        else if (ext == "wmv") pri = 1;
#endif

        if (pri == 0) {
            continue; // Unsupported video format
        }

        auto itBest = bestByBaseName.find(base);
        if (itBest == bestByBaseName.end() || pri > itBest->priority) {
            Candidate c;
            c.fi = fi;
            c.priority = pri;
            bestByBaseName[base] = c;
        }
    }

    for (auto itBest = bestByBaseName.constBegin(); itBest != bestByBaseName.constEnd(); ++itBest) {
        const QFileInfo &fi = itBest->fi;
        const QString filePath = fi.absoluteFilePath();

        // Try to load thumbnail if it exists; otherwise fall back to no icon
        QIcon *ico = nullptr;
        const QString thumb = fi.absolutePath() + QLatin1Char('/') + fi.completeBaseName() + QStringLiteral(".png");
        if (QFile::exists(thumb)) {
            QImageReader imageReader(thumb);
            const QImage sprite = imageReader.read();
            if (!sprite.isNull()) {
                ico = new QIcon(QPixmap::fromImage(sprite));
            } else {
                qDebug() << "warning: skipping thumbnail (failed to read):" << thumb;
            }
        } else {
            qDebug() << "info: no thumbnail found for" << filePath << "- using plain tile";
        }

        QUrl *url = new QUrl(QUrl::fromLocalFile(filePath));
        if (!url->isLocalFile() || !QFile::exists(url->toLocalFile())) {
            qWarning() << "Skipping video with invalid path:" << url->toString();
            delete url;
            delete ico;
            continue;
        }

        qDebug() << "Discovered video (chosen variant):" << url->toLocalFile();
        out.push_back(TheButtonInfo(url, ico));
    }

    if (out.empty()) {
        qWarning() << "No playable videos found in directory:" << dir.absolutePath();
    }

    return out;
}

std::vector<TheButtonInfo> loadVideosFromDefaultLocations() {
    QStringList candidateDirs;
    const QString current = QDir::currentPath();
    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList relativeCandidates = {
        QStringLiteral("videos"),
        QStringLiteral("../videos"),
        QStringLiteral("../../videos"),
        QStringLiteral("src/videos"),
        QStringLiteral("../src/videos")
    };

    for (const QString &rel : relativeCandidates) {
        candidateDirs << QDir::cleanPath(current + QLatin1Char('/') + rel);
    }
    for (const QString &rel : relativeCandidates) {
        candidateDirs << QDir::cleanPath(appDir + QLatin1Char('/') + rel);
    }

    QSet<QString> visited;
    for (const QString &path : candidateDirs) {
        QDir dir(path);
        if (!dir.exists()) {
            continue;
        }
        const QString absolutePath = dir.absolutePath();
        if (visited.contains(absolutePath)) {
            continue;
        }
        visited.insert(absolutePath);
        auto videos = getInfoIn(absolutePath.toStdString());
        if (!videos.empty()) {
            qDebug() << "Loaded videos from default path:" << absolutePath;
            return videos;
        }
    }

    qWarning() << "Unable to locate videos in default locations. Checked:" << candidateDirs;
    return {};
}


int main(int argc, char *argv[]) {

    // let's just check that Qt is operational first
    qDebug() << "Qt version: " << QT_VERSION_STR;

    // create the Qt Application
    QApplication app(argc, argv);

    // Global font settings: use clearer sans-serif font, slightly larger size, similar to Instagram
    {
        QFont baseFont;
#if defined(Q_OS_WIN)
        baseFont = QFont(QStringLiteral("Segoe UI"));
#elif defined(Q_OS_MAC)
        baseFont = QFont(QStringLiteral("SF Pro Text"));
#else
        baseFont = QFont(QStringLiteral("Roboto"));
#endif
        baseFont.setPointSize(11);              // Slightly larger than default, more readable
        baseFont.setStyleStrategy(QFont::PreferAntialias);
        app.setFont(baseFont);
    }

    // collect all the videos in the folder
    std::vector<TheButtonInfo> videos;

    if (argc == 2) {
        videos = getInfoIn(std::string(argv[1]));
    }

    if (videos.empty()) {
        videos = loadVideosFromDefaultLocations();
    }

    if (videos.size() == 0) {

        const int result = QMessageBox::information(
            NULL,
            QString("Tomeo"),
            QString("no videos found! Add command line argument to \"quoted\" file location."));
        exit(-1);
    }

    AccessibilityPreferences accessibilityPrefs;
    QStringList subtitleLanguages = discoverSubtitleLanguages(videos);
    
    // Helper function: select text based on current language
    auto pickText = [&accessibilityPrefs](const QString &zh, const QString &en) {
        return accessibilityPrefs.interfaceLanguage == AppLanguage::Chinese ? zh : en;
    };
    // Voice narration will be initialized after window definition
    // Use global NarrationManager to manage voice narration
    auto narrate = [&](const QString &text) {
        NarrationManager::instance().narrate(text);
    };
    
    auto narratePick = [&](const QString &zh, const QString &en) {
        NarrationManager::instance().narrate(zh, en);
    };
    if (!subtitleLanguages.contains(accessibilityPrefs.subtitleLanguage)) {
        accessibilityPrefs.subtitleLanguage = subtitleLanguages.value(0, QStringLiteral("zh"));
    }

    // the widget that will show the video
    QVideoWidget *videoWidget = new QVideoWidget;
    videoWidget->setObjectName("homeVideo");
    videoWidget->setMinimumHeight(420);

    // the QMediaPlayer which controls the playback
    ThePlayer *player = new ThePlayer;
    player->setVideoOutput(videoWidget);
    player->setContent(nullptr, &videos);

    const QString backgroundUrlDay(":/home/earth.png");
    const QString backgroundUrlNight(":/home/earth1.png");
    
    HomePageWidget *homePage = new HomePageWidget();
    QVBoxLayout *homeLayout = new QVBoxLayout();
    homeLayout->setContentsMargins(32, 32, 32, 32);
    homeLayout->setSpacing(12);
    homePage->setLayout(homeLayout);

    auto *topBar = new QHBoxLayout();
    topBar->setContentsMargins(0, 0, 0, 0);
    topBar->setSpacing(8);

    auto *titleCol = new QVBoxLayout();
    titleCol->setSpacing(2);
    auto *heroTitle = new QLabel(QString::fromUtf8("Antipode: Day Dream Night"));
    heroTitle->setObjectName("heroTitle");
    auto *heroSubtitle = new QLabel(QString::fromUtf8("来自地球另一边的瞬间"));
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
    cardLayout->setContentsMargins(20, 20, 20, 16);
    cardLayout->setSpacing(10);

    auto *cardHeader = new QHBoxLayout();
    cardHeader->setSpacing(10);

    // Use colored ring avatar (similar to Profile page)
    const QIcon *avatarIcon = videos.empty() ? nullptr : videos.front().icon;
    auto *avatarRing = new HomePageAvatarRingWidget(avatarIcon, berealCard);

    auto *identityCol = new QVBoxLayout();
    identityCol->setSpacing(1);
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

    cardHeader->addWidget(avatarRing, 0, Qt::AlignTop);
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
    overlayLayout->setContentsMargins(12, 12, 12, 12);
    overlayLayout->setSpacing(8);

    auto *overlayTop = new QHBoxLayout();
    overlayTop->setSpacing(6);
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
    overlayBottom->setSpacing(8);
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

    // Create independent subtitle display module (below video)
    auto *subtitleFrame = new QFrame(berealCard);
    subtitleFrame->setObjectName("subtitleFrame");
    subtitleFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    subtitleFrame->setMinimumHeight(60);
    subtitleFrame->setMaximumHeight(80);
    auto *subtitleLayout = new QHBoxLayout(subtitleFrame);
    subtitleLayout->setContentsMargins(8, 8, 8, 8);
    subtitleLayout->setSpacing(0);
    
    auto *subtitleLabel = new QLabel("", subtitleFrame);
    subtitleLabel->setObjectName("subtitleLabel");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setWordWrap(false);
    subtitleLabel->setVisible(false);
    subtitleLabel->setMinimumHeight(50);
    subtitleLabel->setTextFormat(Qt::PlainText);
    subtitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    
    subtitleLayout->addWidget(subtitleLabel, 0, Qt::AlignCenter);
    
    cardLayout->addWidget(subtitleFrame);

    auto *metaFooter = new QFrame(berealCard);
    metaFooter->setObjectName("metaFooter");
    auto *metaLayout = new QHBoxLayout(metaFooter);
    metaLayout->setContentsMargins(0, 0, 0, 0);
    metaLayout->setSpacing(8);
    auto *timeLabel = new QLabel(QString::fromUtf8("捕捉时间 · 16:42"), metaFooter);
    timeLabel->setObjectName("metaLabel");
    auto *locationLabel = new QLabel(QString::fromUtf8("位置 · Palermo Rooftop"), metaFooter);
    locationLabel->setObjectName("metaLabel");
    metaLayout->addWidget(timeLabel);
    metaLayout->addWidget(locationLabel);
    metaLayout->addStretch();
    auto *shareNowButton = new QPushButton(QString::fromUtf8("同步到好友"), metaFooter);
    // Default night mode uses high contrast version share_to_friends1.svg
    shareNowButton->setIcon(QIcon(":/icons/icons/share_to_friends1.svg"));
    shareNowButton->setIconSize(QSize(28, 28));
    shareNowButton->setObjectName("shareNowButton");
    metaLayout->addWidget(shareNowButton);
    cardLayout->addWidget(metaFooter);

    auto *progressSlider = new QSlider(Qt::Horizontal, berealCard);
    progressSlider->setObjectName("progressSlider");
    progressSlider->setRange(0, 0);
    progressSlider->setEnabled(!videos.empty());
    progressSlider->setTracking(false);  // Disable tracking, only update on release
    cardLayout->addWidget(progressSlider);
    
    // Track whether the user is currently dragging the slider
    auto *isDragging = new bool(false);

    auto *controlRow = new QHBoxLayout();
    controlRow->setSpacing(8);
    
    // Add volume control slider, place in control button row
    auto *volumeSlider = new QSlider(Qt::Horizontal, berealCard);
    volumeSlider->setObjectName("volumeSlider");
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(100);  // Default volume 100%
    volumeSlider->setEnabled(!videos.empty());

    auto makePillButton = [&](const QString &label, bool checkable = false) {
        auto *btn = new QPushButton(label, berealCard);
        btn->setObjectName("pillButton");
        btn->setCursor(Qt::PointingHandCursor);
        btn->setCheckable(checkable);
        return btn;
    };
    auto makeRecordStyleButton = [&](const QString &iconPath, bool checkable = false) {
        auto *btn = new QPushButton(berealCard);
        btn->setObjectName("recordStyleButton");
        btn->setCursor(Qt::PointingHandCursor);
        btn->setCheckable(checkable);
        btn->setFixedSize(60, 60);
        btn->setStyleSheet(
            "QPushButton#recordStyleButton {"
            "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(255,255,255,0.92), stop:1 rgba(240,248,255,0.92));"
            "  border-radius: 30px;"
            "  border: 3px solid #6CADFF;"
            "}"
            "QPushButton#recordStyleButton:hover {"
            "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(108,173,255,0.2), stop:1 rgba(58,125,255,0.2));"
            "  border-color: #3A7DFF;"
            "}"
            "QPushButton#recordStyleButton:pressed {"
            "  background: rgba(255,255,255,0.8);"
            "}"
        );
        if (!iconPath.isEmpty()) {
            btn->setIcon(QIcon(iconPath));
            btn->setIconSize(QSize(30, 30));
        }
        return btn;
    };
        // Night mode defaults to high contrast icon *1.svg
    auto *playPauseButton = makeRecordStyleButton(QStringLiteral(":/icons/icons/play1.svg"));
    auto *muteButton = makeRecordStyleButton(QString(), true);
    auto *prevMomentButton = makeRecordStyleButton(QStringLiteral(":/icons/icons/prev1.svg"));
    auto *nextMomentButton = makeRecordStyleButton(QStringLiteral(":/icons/icons/next1.svg"));
    
    // Create container with share and repost buttons
    auto *shareRepostContainer = new QFrame(berealCard);
    shareRepostContainer->setObjectName("shareRepostContainer");
    shareRepostContainer->setStyleSheet(
        "QFrame#shareRepostContainer {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(255,255,255,0.92), stop:1 rgba(240,248,255,0.92));"
        "  border-radius: 30px;"
        "  border: 3px solid #6CADFF;"
        "}"
    );
    auto *shareRepostLayout = new QHBoxLayout(shareRepostContainer);
    shareRepostLayout->setContentsMargins(0, 0, 0, 0);
    shareRepostLayout->setSpacing(0);
    
    // Share button (left side)
    auto *shareButton = new QPushButton(shareRepostContainer);
    shareButton->setObjectName("shareRepostButton");
    shareButton->setIcon(QIcon(":/icons/icons/share1.svg"));
    shareButton->setIconSize(QSize(24, 24));
    shareButton->setCursor(Qt::PointingHandCursor);
    shareButton->setStyleSheet(
        "QPushButton#shareRepostButton {"
        "  background: transparent;"
        "  border: none;"
        "  border-top-left-radius: 27px;"
        "  border-bottom-left-radius: 27px;"
        "  border-right: 1px solid rgba(108,173,255,0.3);"
        "  padding: 12px;"
        "}"
        "QPushButton#shareRepostButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(108,173,255,0.2), stop:1 rgba(58,125,255,0.2));"
        "}"
        "QPushButton#shareRepostButton:pressed {"
        "  background: rgba(255,255,255,0.8);"
        "}"
    );
    
    // Repost button (right side)
    auto *repostButton = new QPushButton(shareRepostContainer);
    repostButton->setObjectName("shareRepostButton");
    repostButton->setIcon(QIcon(":/icons/icons/repost1.svg"));
    repostButton->setIconSize(QSize(24, 24));
    repostButton->setCursor(Qt::PointingHandCursor);
    repostButton->setStyleSheet(
        "QPushButton#shareRepostButton {"
        "  background: transparent;"
        "  border: none;"
        "  border-top-right-radius: 27px;"
        "  border-bottom-right-radius: 27px;"
        "  padding: 12px;"
        "}"
        "QPushButton#shareRepostButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(108,173,255,0.2), stop:1 rgba(58,125,255,0.2));"
        "}"
        "QPushButton#shareRepostButton:pressed {"
        "  background: rgba(255,255,255,0.8);"
        "}"
    );
    
    shareRepostLayout->addWidget(shareButton, 1);
    shareRepostLayout->addWidget(repostButton, 1);
    shareRepostContainer->setFixedHeight(60);

    controlRow->addWidget(playPauseButton);
    controlRow->addWidget(muteButton);
    controlRow->addWidget(prevMomentButton);
    controlRow->addWidget(nextMomentButton);
    controlRow->addSpacing(16);  // 音量条左侧间距
    controlRow->addWidget(volumeSlider, 1);  // 音量条放在按钮之间，使用拉伸因子1使其占据剩余空间
    controlRow->addSpacing(16);  // 音量条右侧间距
    controlRow->addWidget(shareRepostContainer);

    cardLayout->addLayout(controlRow);

    auto *reactionRow = new QHBoxLayout();
    reactionRow->setSpacing(6);
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
    commentLayout->setContentsMargins(12, 12, 12, 12);
    commentLayout->setSpacing(6);
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
    auto *commentPlaceholder = new QFrame(berealCard);
    commentPlaceholder->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    commentPlaceholder->setObjectName("commentPlaceholder");
    commentPlaceholder->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    commentPlaceholder->hide();
    cardLayout->addWidget(commentPlaceholder);
    
    // Toggle button to allow users to show/hide the comment panel
    auto *toggleCommentButton = new QCheckBox(QString::fromUtf8("显示互动"), berealCard);
    toggleCommentButton->setObjectName("toggleCommentButton");
    toggleCommentButton->setChecked(true); // Show by default
    toggleCommentButton->setCursor(Qt::PointingHandCursor);
    cardLayout->addWidget(toggleCommentButton, 0, Qt::AlignRight);
    
    // Connect toggle button
    QObject::connect(toggleCommentButton, &QCheckBox::toggled,
                     [commentPanel, commentPlaceholder, berealCard](bool checked) {
                         if (checked) {
                              // Show interaction content
                             commentPlaceholder->hide();
                             commentPanel->show();
                         } else {
                             // 隐藏互动内容，但保持布局稳定
                             // 在隐藏之前获取高度，确保能正确获取尺寸
                             int panelHeight = commentPanel->isVisible() 
                                 ? commentPanel->height() 
                                 : commentPanel->sizeHint().height();
                             if (panelHeight <= 0) {
                                 // 如果高度无效，使用一个默认值
                                 panelHeight = 120;
                             }
                             // 设置占位符高度，保持布局稳定
                             commentPlaceholder->setFixedHeight(panelHeight);
                             commentPlaceholder->show();
                             commentPanel->hide();
                         }
                         // 确保父widget更新布局，但不影响滚动条
                         if (berealCard) {
                             berealCard->updateGeometry();
                         }
                     });

    auto baseReactionPromptText = [&accessibilityPrefs]() {
        return accessibilityPrefs.interfaceLanguage == AppLanguage::Chinese
                   ? QString::fromUtf8("好友的实时反应")
                   : QStringLiteral("Friends reacting live");
    };

    auto updatePlayPauseVisual = [playPauseButton, player]() {
        const bool playing = player->state() == QMediaPlayer::PlayingState;
        // 夜间模式默认使用高对比版本 *1.svg
        const QString iconPath = playing ? QStringLiteral(":/icons/icons/pause1.svg")
                                         : QStringLiteral(":/icons/icons/play1.svg");
        playPauseButton->setIcon(QIcon(iconPath));
        playPauseButton->setIconSize(QSize(30, 30));
    };
    updatePlayPauseVisual();

    auto updateMuteIcon = [muteButton]() {
        const bool checked = muteButton->isChecked();
        // 夜间模式默认使用高对比版本 *1.svg
        const QString iconPath = checked ? QStringLiteral(":/icons/icons/mute1.svg")
                                         : QStringLiteral(":/icons/icons/volume1.svg");
        muteButton->setIcon(QIcon(iconPath));
        muteButton->setIconSize(QSize(30, 30));
    };
    updateMuteIcon();

    homeLayout->addWidget(berealCard, 0, Qt::AlignTop);
    homeLayout->addStretch();

    SubtitleController *subtitleController = new SubtitleController(homePage);
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

    // Different location and time info for 21 videos (top-bottom positions remain consistent)
    struct VideoLocationInfo {
        QString cityZh;
        QString cityEn;
        QString locationZh;
        QString locationEn;
        QString time;
        QString lateHoursZh;
        QString lateHoursEn;
    };
    
    const VideoLocationInfo locationInfos[21] = {
        {"Palermo, Buenos Aires", "Palermo, Buenos Aires", "Palermo Rooftop", "Palermo Rooftop", "16:42", "2 小时", "2 hours"},
        {"Shibuya, Tokyo", "Shibuya, Tokyo", "Shibuya Sky Deck", "Shibuya Sky Deck", "23:15", "1 小时", "1 hour"},
        {"Montmartre, Paris", "Montmartre, Paris", "Sacré-Cœur Steps", "Sacré-Cœur Steps", "14:28", "3 小时", "3 hours"},
        {"Central Park, New York", "Central Park, New York", "Bethesda Fountain", "Bethesda Fountain", "18:33", "1 小时", "1 hour"},
        {"Camden, London", "Camden, London", "Camden Lock Bridge", "Camden Lock Bridge", "20:07", "2 小时", "2 hours"},
        {"Gangnam, Seoul", "Gangnam, Seoul", "COEX Mall Rooftop", "COEX Mall Rooftop", "21:45", "1 小时", "1 hour"},
        {"Kreuzberg, Berlin", "Kreuzberg, Berlin", "Tempelhofer Feld", "Tempelhofer Feld", "19:22", "2 小时", "2 hours"},
        {"Shoreditch, London", "Shoreditch, London", "Brick Lane Market", "Brick Lane Market", "17:50", "3 小时", "3 hours"},
        {"Harajuku, Tokyo", "Harajuku, Tokyo", "Takeshita Street", "Takeshita Street", "22:08", "1 小时", "1 hour"},
        {"Le Marais, Paris", "Le Marais, Paris", "Place des Vosges", "Place des Vosges", "15:12", "2 小时", "2 hours"},
        {"Williamsburg, Brooklyn", "Williamsburg, Brooklyn", "Brooklyn Bridge Park", "Brooklyn Bridge Park", "19:55", "1 小时", "1 hour"},
        {"Itaewon, Seoul", "Itaewon, Seoul", "Namsan Tower View", "Namsan Tower View", "20:30", "2 小时", "2 hours"},
        {"Neukölln, Berlin", "Neukölln, Berlin", "Tempelhof Airport", "Tempelhof Airport", "18:45", "3 小时", "3 hours"},
        {"Palermo, Buenos Aires", "Palermo, Buenos Aires", "Botanical Garden", "Botanical Garden", "16:20", "1 小时", "1 hour"},
        {"Shinjuku, Tokyo", "Shinjuku, Tokyo", "Tokyo Metropolitan Building", "Tokyo Metropolitan Building", "23:40", "2 小时", "2 hours"},
        {"Saint-Germain, Paris", "Saint-Germain, Paris", "Seine Riverbank", "Seine Riverbank", "14:55", "2 小时", "2 hours"},
        {"Greenwich Village, NYC", "Greenwich Village, NYC", "Washington Square", "Washington Square", "18:15", "1 小时", "1 hour"},
        {"Hackney, London", "Hackney, London", "Victoria Park", "Victoria Park", "20:25", "3 小时", "3 hours"},
        {"Hongdae, Seoul", "Hongdae, Seoul", "Hongik University Street", "Hongik University Street", "21:18", "1 小时", "1 hour"},
        {"Prenzlauer Berg, Berlin", "Prenzlauer Berg, Berlin", "Mauerpark", "Mauerpark", "19:05", "2 小时", "2 hours"},
        {"Palermo, Buenos Aires", "Palermo, Buenos Aires", "Recoleta Cemetery", "Recoleta Cemetery", "17:10", "2 小时", "2 hours"}
    };

    auto updateMomentMeta = [&](int index) {
        const auto pick = [&accessibilityPrefs](const QString &zh, const QString &en) {
            return accessibilityPrefs.interfaceLanguage == AppLanguage::Chinese ? zh : en;
        };

        if (videos.empty()) {
            momentLabel->setText(pick(QString::fromUtf8("真实瞬间 · 0 / 0"),
                                      QStringLiteral("Real drops · 0 / 0")));
            captionLabel->setText(pick(QString::fromUtf8("暂无回忆"),
                                       QStringLiteral("No memories yet")));
            timeLabel->setText(pick(QString::fromUtf8("捕捉时间 · --:--"),
                                    QStringLiteral("Captured at · --:--")));
            locationLabel->setText(pick(QString::fromUtf8("位置 · Palermo Rooftop"),
                                        QStringLiteral("Location · Palermo Rooftop")));
            reactionPrompt->setText(baseReactionPromptText());
            narrate(momentLabel->text());
            return;
        }

        if (index < 0 || index >= static_cast<int>(videos.size())) {
            index = 0;
        }

        const QFileInfo fileInfo(videos.at(index).url->toLocalFile());
        const QString clipName = fileInfo.completeBaseName().isEmpty()
                                     ? pick(QString::fromUtf8("未命名片段"),
                                            QStringLiteral("Untitled clip"))
                                     : fileInfo.completeBaseName();

        // Obtain the corresponding location and time information based on the video index
        const int locationIndex = index % 21;
        const VideoLocationInfo &info = locationInfos[locationIndex];

        momentLabel->setText(pick(
            QString::fromUtf8("真实瞬间 · %1 / %2").arg(index + 1).arg(videos.size()),
            QStringLiteral("Real drops · %1 / %2").arg(index + 1).arg(videos.size())));
        captionLabel->setText(pick(
            QString::fromUtf8("“%1 · 与好友同步”").arg(clipName),
            QStringLiteral("“%1 · In sync with friends”").arg(clipName)));
        timeLabel->setText(pick(
            QString::fromUtf8("捕捉时间 · %1").arg(info.time),
            QStringLiteral("Captured at · %1").arg(info.time)));
        locationLabel->setText(pick(
            QString::fromUtf8("位置 · %1").arg(info.locationZh),
            QStringLiteral("Location · %1").arg(info.locationEn)));
        
        // Update dropMeta (displayed at the top of the card)
        dropMeta->setText(pick(
            QString::fromUtf8("距官方提醒 %1 · %2").arg(info.lateHoursZh, info.cityZh),
            QStringLiteral("%1 late · %2").arg(info.lateHoursEn, info.cityEn)));
        
        reactionPrompt->setText(baseReactionPromptText());
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
            selfieBubble->setText(accessibilityPrefs.interfaceLanguage == AppLanguage::Chinese
                                      ? QString::fromUtf8("自拍")
                                      : QStringLiteral("Selfie"));
        }
    };

    // Apply video display parameters (brightness, contrast, saturation) to QVideoWidget
    auto applyVideoDisplayParams = [videoWidget, &accessibilityPrefs]() {
        if (!videoWidget) {
            return;
        }
       
        qreal brightness = (accessibilityPrefs.brightness - 1.0) * 100.0;
        qreal contrast = (accessibilityPrefs.contrast - 1.0) * 100.0;
        qreal saturation = (accessibilityPrefs.saturation - 1.0) * 100.0;
        
        videoWidget->setBrightness(brightness);
        videoWidget->setContrast(contrast);
        videoWidget->setSaturation(saturation);
    };

    auto playVideoAt = [&](int index) {
        if (videos.empty()) {
            return;
        }
        if (index < 0) {
            index = static_cast<int>(videos.size()) - 1;
        }
        currentVideoIndex = index % static_cast<int>(videos.size());
        setVideoStatus(pickText(QString::fromUtf8("加载中..."),
                                QStringLiteral("Loading...")),
                       true);
        updateMomentMeta(currentVideoIndex);
        updateSelfie(currentVideoIndex);
        player->setMedia(*videos.at(currentVideoIndex).url);
        if (subtitleController) {
            subtitleController->setActiveVideo(videos.at(currentVideoIndex).url->toLocalFile());
        }
        // Apply playback speed settings
        player->setPlaybackRate(accessibilityPrefs.playbackRate);
        // Apply video display parameter settings
        applyVideoDisplayParams();
        player->play();
    };

    // Share button functionality: displays the platform selection panel (similar to PublishPage)
    // Needs to be declared before applyHomeLanguage so it can be accessed within the function
    PopupPanel *panelShare = nullptr;
    QPushButton *shareConfirmButton = nullptr;
    
    auto applyHomeLanguage = [&]() {
        const auto pick = [&accessibilityPrefs](const QString &zh, const QString &en) {
            return accessibilityPrefs.interfaceLanguage == AppLanguage::Chinese ? zh : en;
        };

        heroTitle->setText(pick(QString::fromUtf8("对跖点: 白日梦我"),
                                QStringLiteral("Antipode: Day Dream Night")));
        heroSubtitle->setText(pick(
            QString::fromUtf8("来自地球另一边的瞬间"),
            QStringLiteral("Say hi to the far side of the Earth.")));
        // dropMeta is updated by updateMomentMeta based on the current video index
        lateBadge->setText(pick(QString::fromUtf8("双摄延迟 2 小时"),
                                QStringLiteral("Dual-camera delay · 2h")));
        networkBadge->setText(pick(QString::fromUtf8("4G · 68%"),
                                   QStringLiteral("4G · 68%")));
        shareNowButton->setText(pick(QString::fromUtf8("同步到好友"),
                                     QStringLiteral("Share to friends")));
        // The share and repost buttons do not need text updates (icons only)
        reactionPrompt->setText(baseReactionPromptText());
        commentLabel->setText(pick(
            QString::fromUtf8("Luca: 这景色像极了我们上次的深夜实验！"),
            QStringLiteral("Luca: This view feels just like our last midnight experiment!")));
        replyButton->setText(pick(QString::fromUtf8("回复好友"),
                                  QStringLiteral("Reply to friend")));
        toggleCommentButton->setText(pick(QString::fromUtf8("显示互动"),
                                          QStringLiteral("Show interactions")));

        updateMomentMeta(currentVideoIndex);
        updateSelfie(currentVideoIndex);
        updatePlayPauseVisual();
        updateMuteIcon();
        // Update the share panel title and the confirm button text (if already created)
        if (panelShare) {
            panelShare->setTitle(pick(QString::fromUtf8("分享到"), QStringLiteral("Share to")));
            if (shareConfirmButton) {
                shareConfirmButton->setText(pick(QString::fromUtf8("确定"), QStringLiteral("Confirm")));
            }
        }
    };
    applyHomeLanguage();

    QObject::connect(nextMomentButton, &QPushButton::clicked, [&, playVideoAt]() {
        playVideoAt(currentVideoIndex + 1);
    });
    QObject::connect(prevMomentButton, &QPushButton::clicked, [&, playVideoAt]() {
        playVideoAt(currentVideoIndex - 1);
    });
    auto initSharePanel = [&]() {
        if (!panelShare) {
            panelShare = new PopupPanel(berealCard);
            QWidget *container = new QWidget;
            QVBoxLayout *containerLayout = new QVBoxLayout(container);
            containerLayout->setContentsMargins(0, 0, 0, 0);
            containerLayout->setSpacing(16);
            
            QWidget *box = new QWidget;
            QGridLayout *grid = new QGridLayout(box);
            grid->setContentsMargins(10, 10, 10, 10);
            grid->setHorizontalSpacing(16);
            grid->setVerticalSpacing(24);
            QStringList platforms = {"Instagram", "Tinder", "X (Twitter)", "Telegram", "微信", "B站"};
            
            QVector<QCheckBox*> platformCheckboxes;

            for (int i = 0; i < platforms.size(); ++i) {
                const QString &p = platforms.at(i);
                QCheckBox *ck = new QCheckBox(p, box);
                ck->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
                ck->setStyleSheet(
                    "QCheckBox {"
                    "  color: white;"
                    "  font-size: 14px;"
                    "  font-weight: 500;"
                    "  padding: 8px 4px;"
                    "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(108,173,255,0.1), stop:1 rgba(58,125,255,0.15));"
                    "  border: 1px solid #6CADFF;"
                    "  border-radius: 8px;"
                    "}"
                    "QCheckBox::indicator {"
                    "  width: 18px;"
                    "  height: 18px;"
                    "  border: 2px solid #6CADFF;"
                    "  border-radius: 4px;"
                    "  background: transparent;"
                    "}"
                    "QCheckBox::indicator:checked {"
                    "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #6CADFF, stop:1 #3A7DFF);"
                    "  border-color: #FF4F70;"
                    "}"
                    "QCheckBox:hover {"
                    "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(108,173,255,0.2), stop:1 rgba(255,79,112,0.2));"
                    "  border-color: #FF4F70;"
                    "}"
                );
                platformCheckboxes.append(ck);
                int row = i / 2;
                int col = i % 2;
                grid->addWidget(ck, row, col);
            }
            
            containerLayout->addWidget(box);
            
            // add confirm button
            QPushButton *confirmButton = new QPushButton(container);
            confirmButton->setText(pickText(QString::fromUtf8("确定"), QStringLiteral("Confirm")));
            shareConfirmButton->setStyleSheet(
                "QPushButton {"
                "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #6CADFF, stop:1 #3A7DFF);"
                "  color: white;"
                "  border: 2px solid #6CADFF;"
                "  border-radius: 12px;"
                "  padding: 12px 24px;"
                "  font-size: 16px;"
                "  font-weight: 600;"
                "}"
                "QPushButton:hover {"
                "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3A7DFF, stop:1 #6CADFF);"
                "  border-color: #FF4F70;"
                "}"
                "QPushButton:pressed {"
                "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2A6DEF, stop:1 #5C9DFF);"
                "}"
            );
            shareConfirmButton->setCursor(Qt::PointingHandCursor);
            
            QObject::connect(shareConfirmButton, &QPushButton::clicked, [&, platformCheckboxes, setVideoStatus, narrate, pickText]() {
                QStringList selectedPlatforms;
                for (QCheckBox *ck : platformCheckboxes) {
                    if (ck->isChecked()) {
                        selectedPlatforms.append(ck->text());
                    }
                }
                
                if (selectedPlatforms.isEmpty()) {
                    const QString noSelectionMsg = pickText(
                        QString::fromUtf8("请至少选择一个平台"),
                        QStringLiteral("Please select at least one platform"));
                    setVideoStatus(noSelectionMsg, true);
                    narrate(noSelectionMsg);
                    QTimer::singleShot(2000, [setVideoStatus]() {
                        setVideoStatus(QString(), false);
                    });
                } else {
                    const QString shareMsg = pickText(
                        QString::fromUtf8("已分享到: %1 ✅").arg(selectedPlatforms.join(", ")),
                        QStringLiteral("Shared to: %1 ✅").arg(selectedPlatforms.join(", ")));
                    setVideoStatus(shareMsg, true);
                    narrate(shareMsg);
                    panelShare->hidePanel();
                    QTimer::singleShot(2000, [setVideoStatus]() {
                        setVideoStatus(QString(), false);
                    });
                }
            });
            
            containerLayout->addWidget(shareConfirmButton);

            panelShare->setContent(container);
        }
        const auto pick = [&accessibilityPrefs](const QString &zh, const QString &en) {
            return accessibilityPrefs.interfaceLanguage == AppLanguage::Chinese ? zh : en;
        };
        panelShare->setTitle(pick(QString::fromUtf8("分享到"), QStringLiteral("Share to")));
        // Update the confirm button text (if created)
        if (shareConfirmButton) {
            shareConfirmButton->setText(pick(QString::fromUtf8("确定"), QStringLiteral("Confirm")));
        }
    };
    
    QObject::connect(shareButton, &QPushButton::clicked, [&]() {
        initSharePanel();
        panelShare->showPanel();
    });

    // Repost button function: directly shows 'Share Successful'
    QObject::connect(repostButton, &QPushButton::clicked, [&, setVideoStatus, narrate, pickText]() {
        const QString repostMessage = pickText(
            QString::fromUtf8("转发成功 ✅"),
            QStringLiteral("Reposted successfully ✅"));
        setVideoStatus(repostMessage, true);
        narrate(repostMessage);
        QTimer::singleShot(2000, [setVideoStatus]() {
            setVideoStatus(QString(), false);
        });
    });

    QObject::connect(shareNowButton, &QPushButton::clicked, [&, narrate, pickText, berealCard]() {
        const QString shareMessage = pickText(
            QString::fromUtf8("你的双摄瞬间已经推送给好友 ✅"),
            QStringLiteral("Your dual-camera drop has been sent to friends ✅"));
        QMessageBox::information(
            berealCard,
            pickText(QString::fromUtf8("同步完成"),
                    QStringLiteral("Synced")),
            shareMessage);
        narrate(shareMessage);
    });

    QObject::connect(replyButton, &QPushButton::clicked, [&, narrate, pickText, berealCard]() {
        const QString replyMessage = pickText(
            QString::fromUtf8("准备发一条\"真实\"评论吧！"),
            QStringLiteral("Get ready to drop a truly real reply!"));
        QMessageBox::information(
            berealCard,
            pickText(QString::fromUtf8("回复好友"),
                    QStringLiteral("Reply to friend")),
            replyMessage);
        narrate(replyMessage);
    });

    QObject::connect(playPauseButton, &QPushButton::clicked, [player]() {
        if (player->state() == QMediaPlayer::PlayingState) {
            player->pause();
        } else {
            player->play();
        }
    });

    QObject::connect(player, &QMediaPlayer::stateChanged,
                     playPauseButton, [updatePlayPauseVisual, narrate, player, &accessibilityPrefs, pickText](QMediaPlayer::State state) {
        updatePlayPauseVisual();
        if (state == QMediaPlayer::PausedState || state == QMediaPlayer::StoppedState) {
            const QString playLabel = pickText(
                QString::fromUtf8("播放"),
                QStringLiteral("Play"));
            narrate(playLabel);
        }
    });

    // Mute button connection (synchronized with the volume bar, see the volume control bar connection below)

    // Slider drag handling - fix the issue of returning to the original position after dragging
    QObject::connect(progressSlider, &QSlider::sliderPressed, [isDragging]() {
        *isDragging = true;
    });
    QObject::connect(progressSlider, &QSlider::sliderMoved, [player, isDragging](int value) {
        if (*isDragging) {
            player->setPosition(static_cast<qint64>(value));
        }
    });
    QObject::connect(progressSlider, &QSlider::sliderReleased, [player, progressSlider, isDragging]() {
        *isDragging = false;
        const int sliderPos = progressSlider->sliderPosition();
        progressSlider->setValue(sliderPos);
        player->setPosition(static_cast<qint64>(sliderPos));
    });

    QObject::connect(player, &QMediaPlayer::durationChanged, [progressSlider](qint64 duration) {
        const int sliderMax = duration > 0
                                  ? static_cast<int>(std::min<qint64>(duration, std::numeric_limits<int>::max()))
                                  : 0;
        progressSlider->setMaximum(sliderMax);
        progressSlider->setEnabled(sliderMax > 0);
    });
    QObject::connect(player, &QMediaPlayer::positionChanged, [progressSlider, isDragging](qint64 position) {
        // Only update when the user is not actively dragging
        if (!(*isDragging) && !progressSlider->isSliderDown() && !progressSlider->signalsBlocked()) {
            const int sliderPos = static_cast<int>(std::min<qint64>(position, std::numeric_limits<int>::max()));
            progressSlider->setValue(sliderPos);
        }
    });
    
    // Volume control bar connection
    QObject::connect(volumeSlider, &QSlider::valueChanged, [player, muteButton](int value) {
        player->setVolume(value);
        // If the volume is set to 0, mute automatically; if it increases from 0, unmute
        // Use blockSignals to avoid triggering a loop
        if (value == 0 && !muteButton->isChecked()) {
            muteButton->blockSignals(true);
            muteButton->setChecked(true);
            muteButton->blockSignals(false);
        } else if (value > 0 && muteButton->isChecked()) {
            muteButton->blockSignals(true);
            muteButton->setChecked(false);
            muteButton->blockSignals(false);
        }
    });
    // Sync mute button and volume bar
    QObject::connect(muteButton, &QPushButton::toggled, [player, volumeSlider, updateMuteIcon](bool checked) {
        player->setMuted(checked);
        updateMuteIcon();
        // If unmuted, restore the previous volume (if it's currently 0, set it to 50)
        if (!checked && volumeSlider->value() == 0) {
            volumeSlider->blockSignals(true);
            volumeSlider->setValue(50);
            volumeSlider->blockSignals(false);
        }
    });

    // Connect to the subtitle controller
    QObject::connect(player, &QMediaPlayer::positionChanged, subtitleController, &SubtitleController::handlePosition);
    QObject::connect(subtitleController, &SubtitleController::subtitleChanged,
                     [subtitleLabel, subtitleFrame, &accessibilityPrefs, &subtitleAvailable, pickText](const QString &text) {
        if (!subtitleLabel || !subtitleFrame) {
            return;
        }
        if (!accessibilityPrefs.subtitlesEnabled) {
            subtitleLabel->hide();
            subtitleFrame->hide();
            return;
        }
        const QString unavailableText = pickText(
            QString::fromUtf8("此片段暂无字幕"),
            QStringLiteral("No subtitles for this clip"));
        if (text.isEmpty()) {
            if (!subtitleAvailable) {
                subtitleLabel->setText(unavailableText);
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
        }
    });
    QObject::connect(subtitleController, &SubtitleController::subtitleAvailabilityChanged,
                     [subtitleLabel, subtitleFrame, &accessibilityPrefs, &subtitleAvailable, pickText](bool available) {
        subtitleAvailable = available;
        if (!subtitleLabel || !subtitleFrame) {
            return;
        }
        if (!accessibilityPrefs.subtitlesEnabled) {
            subtitleLabel->hide();
            subtitleFrame->hide();
            return;
        }
        const QString unavailableText = pickText(
            QString::fromUtf8("此片段暂无字幕"),
            QStringLiteral("No subtitles for this clip"));
        if (!available) {
            subtitleLabel->setText(unavailableText);
            subtitleLabel->adjustSize();
            subtitleLabel->show();
            subtitleFrame->show();
        } else if (subtitleLabel->text() == unavailableText) {
            subtitleLabel->clear();
            subtitleLabel->hide();
            subtitleFrame->hide();
        }
    });

    for (auto *btn : reactionButtons) {
        QObject::connect(btn, &QPushButton::clicked,
                         [reactionPrompt, emoji = btn->text(), baseReactionPromptText, narrate, pickText]() {
                             const QString reactionText = pickText(
                                 QString::fromUtf8("你刚刚发送了 %1").arg(emoji),
                                 QStringLiteral("You just reacted with %1").arg(emoji));
                             reactionPrompt->setText(reactionText);
                             narrate(reactionText);
                             QTimer::singleShot(2000, [reactionPrompt, baseReactionPromptText]() {
                                 reactionPrompt->setText(baseReactionPromptText());
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
                     [&setVideoStatus, pickText](QMediaPlayer::MediaStatus status) {
                         switch (status) {
                         case QMediaPlayer::LoadingMedia:
                         case QMediaPlayer::BufferedMedia:
                         case QMediaPlayer::StalledMedia:
                             setVideoStatus(pickText(
                                                QString::fromUtf8("加载中..."),
                                                QStringLiteral("Loading...")),
                                            true);
                             break;
                         case QMediaPlayer::EndOfMedia:
                             setVideoStatus(pickText(
                                                QString::fromUtf8("播放结束"),
                                                QStringLiteral("Playback finished")),
                                            true);
                             break;
                         case QMediaPlayer::LoadedMedia:
                             setVideoStatus(QString(), false);
                             break;
                         default:
                             break;
                         }
                     });

    QObject::connect(player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error),
                     [player, setVideoStatus, pickText](QMediaPlayer::Error error) {
                         if (error == QMediaPlayer::NoError) {
                             return;
                         }
                         const QString errText = player->errorString().isEmpty()
                                                     ? pickText(
                                                           QString::fromUtf8("无法播放此视频"),
                                                           QStringLiteral("Cannot play this video"))
                                                     : player->errorString();
                         setVideoStatus(pickText(
                                            QString::fromUtf8("播放失败: %1").arg(errText),
                                            QStringLiteral("Playback failed: %1").arg(errText)),
                                        true);
                     });

    QObject::connect(player, &QMediaPlayer::stateChanged, [setVideoStatus](QMediaPlayer::State state) {
        if (state == QMediaPlayer::PlayingState) {
            setVideoStatus(QString(), false);
        }
    });

    // Initialize the LanguageManager to ensure the default language aligns with accessibility settings
    LanguageManager::instance().setLanguage(accessibilityPrefs.interfaceLanguage);

    ProfilePage *profilePage = new ProfilePage(videos);
    profilePage->setLanguage(accessibilityPrefs.interfaceLanguage);
    profilePage->setHighContrastMode(accessibilityPrefs.colorBlindPaletteEnabled);
    
    ChatPage *chatPage = new ChatPage();
    chatPage->setLanguage(accessibilityPrefs.interfaceLanguage);
    chatPage->setHighContrastMode(accessibilityPrefs.colorBlindPaletteEnabled);
    FriendsPage *friendsPage = new FriendsPage(videos);  // Pass video list
    RecordPage *recordPage = new RecordPage();
    PublishPage *publishPage = new PublishPage();

    // Create a scrollable area to display the background image
    QScrollArea *homeScrollArea = new QScrollArea();
    homeScrollArea->setWidget(homePage);
    homeScrollArea->setWidgetResizable(true);
    homeScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    homeScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    homeScrollArea->setStyleSheet(
        "QScrollArea {"
        "  border: none;"
        "  background: transparent;"
        "}"
        "QScrollBar:vertical {"
        "  background: rgba(4,10,20,0.5);"
        "  width: 12px;"
        "  border-radius: 6px;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: rgba(93,155,255,0.6);"
        "  border-radius: 6px;"
        "  min-height: 30px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "  background: rgba(93,155,255,0.8);"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "  height: 0px;"
        "}"
    );
    
    // Give homePage enough height so the background can scroll slightly
    // Earth artwork focuses on the lower half, so we add extra vertical space
    // Set a sufficient minimum height to ensure the scrollbar is still available even when interactive content is hidden
    homePage->setMinimumHeight(1200); // Allow scrolling, but not for too long
    homePage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    
    // Connect the scrollbar signal so that the color wheel changes with scrolling
    QObject::connect(homeScrollArea->verticalScrollBar(), &QAbstractSlider::valueChanged, 
                     [homePage, homeScrollArea](int value) {
        // Convert scroll position to angle (0-360 degrees)
        const int maxScroll = homeScrollArea->verticalScrollBar()->maximum();
        if (maxScroll > 0) {
            const qreal angle = (static_cast<qreal>(value) / maxScroll) * 360.0;
            homePage->setGradientAngle(angle);
        } else {
            homePage->setGradientAngle(0.0);
        }
    });
    
    QStackedWidget *stackedPages = new QStackedWidget();
    stackedPages->addWidget(homeScrollArea);  // 0: Home (within a scroll area)
    stackedPages->addWidget(friendsPage);   // 1: Friends
    stackedPages->addWidget(recordPage);    // 2: Record
    stackedPages->addWidget(chatPage);      // 3: Chat
    stackedPages->addWidget(profilePage);   // 4: Profile
    stackedPages->addWidget(publishPage);   // 5: Publish 

    QWidget window;
    window.setObjectName("appRoot");
    QVBoxLayout *top = new QVBoxLayout();
    top->setContentsMargins(28, 24, 28, 32);
    top->setSpacing(16);
    window.setLayout(top);
    window.setWindowTitle("tomeo");
    window.setMinimumSize(420, 720);

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    // Initialize the voice broadcast manager
    NarrationManager::instance().initialize(&window);
    NarrationManager::instance().setEnabled(accessibilityPrefs.narrationEnabled);
    NarrationManager::instance().setLanguage(accessibilityPrefs.interfaceLanguage);

    auto buildNightStyle = [&]() {
        return QStringLiteral(
                   "QWidget#appRoot {"
                   "  background-color: #00040d;"
                   "}"
                   "QWidget#homePage {"
                   "  background: qradialgradient(cx:0.25, cy:0.2, radius:1.25,"
                   "    stop:0 #091230, stop:0.5 #030918, stop:1 #00040a);"
                   "}"
                   "QLabel#heroTitle { font-size: 22px; font-weight: 700; color: white; }"
                   "QLabel#heroSubtitle { color: #6f84b8; }"
                   "QPushButton#settingsButton {"
                   "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(15,30,55,0.85), stop:1 rgba(25,50,95,0.85));"
                   "  color: white;"
                   "  border: 1px solid rgba(63,134,255,0.45);"
                   "  border-radius: 22px;"
                   "  padding: 10px 22px;"
                   "  font-weight: 600;"
                   "}"
                   "QPushButton#settingsButton:hover {"
                   "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(58,125,255,0.65), stop:1 rgba(108,173,255,0.65));"
                   "  border-color: #6CADFF;"
                   "}"
                   "QFrame#berealCard {"
                   "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(8,20,60,0.95), stop:1 rgba(12,40,118,0.9));"
                   "  border-radius: 40px;"
                   "  border: 2px solid rgba(58,125,255,0.45);"
                   "  outline: 1px solid rgba(2,4,12,0.6);"
                   "}"
                   "QLabel#displayName { color: white; font-size: 18px; font-weight: 700; }"
                   "QLabel#dropMeta { color: #8aa7d9; }"
                   "QLabel#momentLabel { color: #9db6ff; font-weight: 600; }"
                   "QFrame#captureFrame { background: black; border-radius: 32px; }"
                   "QVideoWidget#homeVideo { border-radius: 32px; background-color: black; }"
                   "QWidget#captureOverlay { border-radius: 32px; }"
                   "QFrame#subtitleFrame { background: transparent; }"
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
                   "QLabel#lateBadge {"
                   "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(58,125,255,0.25), stop:1 rgba(108,173,255,0.25));"
                   "  color: #93caff;"
                   "  border: 1px solid rgba(108,173,255,0.4);"
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
                   "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(5,10,22,0.85), stop:1 rgba(8,20,50,0.85));"
                   "  color: white;"
                   "  border: 2px solid rgba(157,182,255,0.5);"
                   "  border-radius: 28px;"
                   "  font-weight: 600;"
                   "}"
                   "QFrame#metaFooter { color: #8aa7d9; }"
                   "QLabel#metaLabel { color: #8aa7d9; }"
                   "QPushButton#shareNowButton {"
                   "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3A7DFF, stop:0.5 #6CADFF, stop:1 #3A7DFF);"
                   "  color: white;"
                   "  border: 2px solid #6CADFF;"
                   "  border-radius: 20px;"
                   "  padding: 8px 20px;"
                   "  font-weight: 700;"
                   "}"
                   "QPushButton#shareNowButton:hover {"
                   "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #6CADFF, stop:0.5 #3A7DFF, stop:1 #6CADFF);"
                   "  border-color: #BFBFBF;"
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
                   "QSlider#volumeSlider::groove {"
                   "  height: 4px;"
                   "  background: rgba(255,255,255,0.18);"
                   "  border-radius: 2px;"
                   "}"
                   "QSlider#volumeSlider::handle {"
                   "  width: 16px;"
                   "  background: rgba(255,255,255,0.9);"
                   "  border-radius: 8px;"
                   "  margin: -6px 0;"
                   "}"
                   "QSlider#volumeSlider::sub-page {"
                   "  background: rgba(93,155,255,0.85);"
                   "  border-radius: 2px;"
                   "}"
                   "QPushButton#pillButton {"
                   "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(24,72,156,0.85), stop:1 rgba(58,125,255,0.85));"
                   "  color: white;"
                   "  border: 2px solid #6CADFF;"
                   "  border-radius: 18px;"
                   "  padding: 10px 18px;"
                   "  font-weight: 600;"
                   "}"
                   "QPushButton#pillButton:hover {"
                   "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(58,125,255,0.95), stop:1 rgba(108,173,255,0.95));"
                   "  border-color: #3A7DFF;"
                   "}"
                   "QPushButton#pillButton:checked {"
                   "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(255,79,112,0.3), stop:1 rgba(108,173,255,0.3));"
                   "  border-color: #FF4F70;"
                   "  color: #9db6ff;"
                   "}"
                   "QLabel#reactionPrompt { color: #8aa7d9; }"
                   "QPushButton#reactionButton {"
                   "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(6,16,34,0.9), stop:1 rgba(13,13,13,0.9));"
                   "  color: white;"
                   "  border: 2px solid #6CADFF;"
                   "  border-radius: 16px;"
                   "  padding: 6px 12px;"
                   "  font-size: 18px;"
                   "}"
                   "QPushButton#reactionButton:hover {"
                   "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(58,125,255,0.3), stop:1 rgba(108,173,255,0.3));"
                   "  border-color: #FF4F70;"
                   "}"
                   "QFrame#commentPanel {"
                   "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(4,12,26,0.95), stop:1 rgba(8,20,50,0.9));"
                   "  border-radius: 26px;"
                   "  border: 1px solid rgba(63,134,255,0.35);"
                   "  outline: 1px solid rgba(10,18,40,0.6);"
                   "}"
                   "QLabel#captionLabel { color: white; font-size: 16px; font-weight: 600; }"
                   "QLabel#commentLabel { color: #9fb1d6; }"
                   "QPushButton#replyButton {"
                   "  background: transparent;"
                   "  color: #9db6ff;"
                   "  border: none;"
                   "  font-weight: 600;"
                   "}"
                   "QCheckBox#toggleCommentButton {"
                   "  color: #8aa7d9;"
                   "  font-size: 14px;"
                   "  font-weight: 600;"
                   "  padding: 8px 12px;"
                   "}"
                   "QCheckBox#toggleCommentButton::indicator {"
                   "  width: 18px;"
                   "  height: 18px;"
                   "  border: 2px solid #8aa7d9;"
                   "  border-radius: 4px;"
                   "  background: transparent;"
                   "}"
                   "QCheckBox#toggleCommentButton::indicator:checked {"
                   "  background: rgba(93,155,255,0.85);"
                   "  border-color: rgba(93,155,255,0.85);"
                   "}"
                   "QCheckBox#toggleCommentButton:hover { color: #9db6ff; }"
                   "QFrame#floatingNav {"
                   "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(4,10,20,0.92), stop:1 rgba(8,20,40,0.92));"
                   "  border-radius: 28px;"
                   "  border: 1px solid rgba(47,141,255,0.35);"
                   "  outline: 1px solid rgba(10,18,40,0.6);"
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
                   "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(58,125,255,0.3), stop:1 rgba(108,173,255,0.3));"
                   "  border: 1px solid rgba(108,173,255,0.5);"
                   "}"
        );
    };

    auto buildDayStyle = [&]() {
        return QStringLiteral(
            "QWidget#appRoot {"
            "  background-color: #f4f7ff;"
            "}"
                   "QWidget#homePage {"
            "  background: qradialgradient(cx:0.25, cy:0.2, radius:1.25,"
            "    stop:0 #e8f0ff, stop:0.5 #dde8f5, stop:1 #f4f7ff);"
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
                   "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(240,245,255,0.95), stop:1 rgba(220,235,255,0.9));"
                   "  border-radius: 40px;"
                   "  border: 2px solid rgba(58,125,255,0.35);"
                   "}"
                   "QLabel#displayName { color: #0c1b33; }"
                   "QLabel#dropMeta { color: #5f6d8c; }"
                   "QLabel#momentLabel { color: #3353b3; font-weight: 600; }"
                   "QFrame#captureFrame { background: #000; border-radius: 32px; }"
                   "QVideoWidget#homeVideo { border-radius: 32px; }"
                   "QFrame#subtitleFrame { background: transparent; }"
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
                   "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3A7DFF, stop:0.5 #6CADFF, stop:1 #3A7DFF);"
                   "  color: white;"
                   "  border: 2px solid #6CADFF;"
                   "  border-radius: 20px;"
                   "  padding: 8px 20px;"
                   "  font-weight: 700;"
                   "}"
                   "QPushButton#shareNowButton:hover {"
                   "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #6CADFF, stop:0.5 #3A7DFF, stop:1 #6CADFF);"
                   "  border-color: #BFBFBF;"
                   "}"
                   "QSlider#progressSlider::groove { background: rgba(32,50,90,0.15); height: 4px; border-radius: 2px; }"
                   "QSlider#progressSlider::handle { width: 16px; background: #3353b3; border-radius: 8px; margin: -6px 0; }"
                   "QSlider#progressSlider::sub-page { background: #6f8dff; border-radius: 2px; }"
                   "QSlider#volumeSlider::groove { background: rgba(32,50,90,0.15); height: 4px; border-radius: 2px; }"
                   "QSlider#volumeSlider::handle { width: 16px; background: #3353b3; border-radius: 8px; margin: -6px 0; }"
                   "QSlider#volumeSlider::sub-page { background: #6f8dff; border-radius: 2px; }"
                   "QPushButton#pillButton {"
                   "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(191,191,191,0.2), stop:1 rgba(108,173,255,0.3));"
                   "  color: #0c1b33;"
                   "  border: 2px solid #6CADFF;"
                   "  border-radius: 18px;"
                   "  padding: 10px 18px;"
                   "  font-weight: 600;"
                   "}"
                   "QPushButton#pillButton:hover {"
                   "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(108,173,255,0.4), stop:1 rgba(58,125,255,0.5));"
                   "  border-color: #3A7DFF;"
                   "}"
                   "QPushButton#pillButton:checked {"
                   "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(255,79,112,0.2), stop:1 rgba(108,173,255,0.3));"
                   "  border-color: #FF4F70;"
                   "  color: #20324f;"
                   "}"
                   "QLabel#reactionPrompt { color: #5f6d8c; }"
                   "QPushButton#reactionButton {"
                   "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(255,255,255,0.95), stop:1 rgba(191,191,191,0.8));"
                   "  color: #20324f;"
                   "  border: 2px solid #6CADFF;"
                   "  border-radius: 16px;"
                   "  padding: 6px 12px;"
                   "  font-size: 18px;"
                   "}"
                   "QPushButton#reactionButton:hover {"
                   "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(108,173,255,0.3), stop:1 rgba(58,125,255,0.4));"
                   "  border-color: #FF4F70;"
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
                   "QCheckBox#toggleCommentButton {"
                   "  color: #5f6d8c;"
                   "  font-size: 14px;"
                   "  font-weight: 600;"
                   "  padding: 8px 12px;"
                   "}"
                   "QCheckBox#toggleCommentButton::indicator {"
                   "  width: 18px;"
                   "  height: 18px;"
                   "  border: 2px solid #5f6d8c;"
                   "  border-radius: 4px;"
                   "  background: transparent;"
                   "}"
                   "QCheckBox#toggleCommentButton::indicator:checked {"
                   "  background: #3353b3;"
                   "  border-color: #3353b3;"
                   "}"
                   "QCheckBox#toggleCommentButton:hover { color: #20324f; }"
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
        );
    };

    auto buildColorBlindStyle = [&]() {
        return QStringLiteral(
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
            // High contrast: reply button with black background and yellow text
            "QPushButton#replyButton { background-color: #000000; color: #f4c430; border: 2px solid #f4c430; border-radius: 18px; font-weight: 700; padding: 6px 18px; }"
            "QPushButton#replyButton:hover { background-color: #111111; border-color: #ffd700; }"
            // High contrast: Share with friends (shareNowButton in metaFooter) module with black background and yellow text
            "QPushButton#shareNowButton { background-color: #000000; color: #f4c430; border: 2px solid #f4c430; border-radius: 20px; padding: 8px 20px; font-weight: 700; }"
            "QPushButton#shareNowButton:hover { background-color: #111111; border-color: #ffd700; }"
            "QLabel#metaLabel { color: #f4c430; }"
            "QFrame#floatingNav { background-color: #000000; border: 2px solid #f4c430; border-radius: 28px; }"
            // In high contrast mode: navigation button text is always on a pure black background, and the size is consistent
            "QPushButton#navButton {"
            "  background-color: #000000;"
            "  color: #f4c430;"
            "  border: 2px solid #f4c430;"
            "  border-radius: 18px;"
            "  font-weight: 700;"
            "  padding: 10px 18px;"
            "}"
            "QPushButton#navButton:checked {"
            "  background-color: #000000;"
            "  color: #f4c430;"
            "  border: 3px solid #ffd700;"
            "  border-radius: 18px;"
            "}"
            "QSlider#progressSlider::groove { background: #f4c430; height: 4px; }"
            "QSlider#progressSlider::handle { width: 16px; background: #ffffff; border-radius: 8px; margin: -6px 0; }"
            "QSlider#volumeSlider::groove { background: #f4c430; height: 4px; }"
            "QSlider#volumeSlider::handle { width: 16px; background: #ffffff; border-radius: 8px; margin: -6px 0; }"
            "QFrame#controlsBar { background-color: rgba(0,0,0,0.9); border: 2px solid #f4c430; border-radius: 18px; }"
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

    bool nightMode = true;
    auto applyTheme = [&](bool night) {
        nightMode = night;
        const bool highContrast = accessibilityPrefs.colorBlindPaletteEnabled;
        if (highContrast) {
            window.setStyleSheet(buildColorBlindStyle());
        } else if (night) {
            window.setStyleSheet(buildNightStyle());
        } else {
            window.setStyleSheet(buildDayStyle());
        }

        if (night && !highContrast) {
            homePage->setBackgroundImage(backgroundUrlNight);
        } else if (!night && !highContrast) {
            homePage->setBackgroundImage(backgroundUrlDay);
        }
        // Update the theme of ProfilePage and ChatPage
        if (profilePage) {
            profilePage->setHighContrastMode(highContrast);
            if (!highContrast) {
                profilePage->setDayMode(!night);
            }
        }
        if (chatPage && chatPage->metaObject()->indexOfMethod("setHighContrastMode(bool)") >= 0) {
            chatPage->setHighContrastMode(highContrast);
        }
        if (friendsPage) {
            friendsPage->setHighContrastMode(highContrast);
            if (!highContrast) {
                friendsPage->setDayMode(!night);
            }
        }
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
        QString zh;
        QString en;
        int index;
        QPushButton *button;

        NavSpec(const QString &zhLabel,
                const QString &enLabel,
                int idx)
            : zh(zhLabel), en(enLabel), index(idx), button(nullptr) {}
    };
    std::vector<NavSpec> navSpecs = {
        {QString::fromUtf8("主页"), QStringLiteral("Home"), 0},
        {QString::fromUtf8("朋友圈"), QStringLiteral("Friends"), 1},
        {QString::fromUtf8("录制"), QStringLiteral("Record"), 2},
        {QString::fromUtf8("聊天"), QStringLiteral("Chat"), 3},
        {QString::fromUtf8("个人"), QStringLiteral("Profile"), 4}
    };

    QPushButton *firstButton = nullptr;
    for (auto &spec : navSpecs) {
        auto *button = new QPushButton(spec.en, floatingNav);
        button->setObjectName("navButton");
        button->setCheckable(true);
        button->setCursor(Qt::PointingHandCursor);
        // Ensure that the five buttons are the same size: uniform height + equal horizontal spacing
        button->setMinimumHeight(44);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        navGroup->addButton(button, spec.index);
        floatingLayout->addWidget(button);
        spec.button = button;

        const int targetIndex = spec.index;
        QObject::connect(button, &QPushButton::toggled, stackedPages, [stackedPages, targetIndex, &spec, &accessibilityPrefs](bool checked) {
            if (checked) {
                stackedPages->setCurrentIndex(targetIndex);
                // Broadcast page switch
                QString pageName = accessibilityPrefs.interfaceLanguage == AppLanguage::Chinese ? spec.zh : spec.en;
                NarrationManager::instance().narrate(pageName);
            }
        });

        // Handle navigation bar button references
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

    auto applyNavLanguage = [&navSpecs, &accessibilityPrefs]() {
        for (auto &spec : navSpecs) {
            if (spec.button) {
                spec.button->setText(accessibilityPrefs.interfaceLanguage == AppLanguage::Chinese ? spec.zh : spec.en);
            }
        }
    };
    applyNavLanguage();

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

    // Page navigation logic: Record -> Publish
    QObject::connect(recordPage, &RecordPage::recordingFinished, [&](){
        stackedPages->setCurrentIndex(5);  // Navigate to the Publish page
        // voice narration
        NarrationManager::instance().narrate(
            QString::fromUtf8("录制完成，进入发布页面"),
            "Recording finished, entering publish page"
        );
    });

    // Navigation: Publish -> Friends
    QObject::connect(publishPage, &PublishPage::sendPressed,
                     [&](const QString &thumb){
                         friendsPage->addNewPost(thumb);
                         if (friendsNavButton) {
                             friendsNavButton->setChecked(true);
                         } else {
                             stackedPages->setCurrentIndex(1);  // Switch to Friends page
                         }
                         // voice narration
                         NarrationManager::instance().narrate(
                             QString::fromUtf8("发布成功，已添加到朋友圈"),
                             "Published successfully, added to friends feed"
                         );
                     });

    // Page navigation logic: Publish -> Record (Back)
    QObject::connect(publishPage, &PublishPage::backToRecord, [&](){
        stackedPages->setCurrentIndex(2);  // Record 
        if (recordNavButton) {
            recordNavButton->setChecked(true);
        }
    });

    // Page navigation logic: Record -> Publish (select draft)
    QObject::connect(recordPage, &RecordPage::draftSelected, [&](const QString &draftText){
        publishPage->loadDraft(draftText);
        stackedPages->setCurrentIndex(5);  // Go to the Publish page
        // voice narration
        NarrationManager::instance().narrate(
            QString::fromUtf8("已选择草稿，进入发布页面"),
            "Draft selected, entering publish page"
        );
    });

    // Video playback request on the Profile page
    if (profilePage) {
        QObject::connect(profilePage, &ProfilePage::playVideoRequested, &window, [&, homeNavButton](int index) {
            if (homeNavButton && !homeNavButton->isChecked()) {
                homeNavButton->setChecked(true);
            } else {
                stackedPages->setCurrentIndex(0);
            }
            playVideoAt(index);
            // Voice Broadcast
            NarrationManager::instance().narrate(
                QString::fromUtf8("播放视频 %1").arg(index + 1),
                QString("Playing video %1").arg(index + 1)
            );
        });
    }

    // Friends page can request a jump to Profile
    QObject::connect(friendsPage, &FriendsPage::goToProfile, [&](const QString &username) {
        if (profileNavButton) {
            profileNavButton->setChecked(true);
        } else {
            stackedPages->setCurrentIndex(4);  // Profile page
        }
        
        // voice narration
        NarrationManager::instance().narrate(
            QString::fromUtf8("打开 %1 的主页").arg(username),
            QString("Open %1's profile").arg(username)
        );
        
        // Set User Information
        if (profilePage) {
            // Generate user information
            QString displayName = username;
            if (!displayName.isEmpty()) {
                displayName[0] = displayName[0].toUpper();
            }
            
            // Generate different bios based on the username (using hashing to ensure consistency)
            uint hash = qHash(username);
            QStringList bios = {
                "Creator of amazing content • Sharing life moments",
                "Photography enthusiast • Capturing beautiful moments",
                "Content creator • Living life to the fullest",
                "Artist • Expressing creativity through videos",
                "Adventurer • Exploring the world one video at a time",
                "Video storyteller • Documenting daily adventures",
                "Creative mind • Always exploring new perspectives",
                "Life enthusiast • Capturing moments that matter"
            };
            QString bio = bios.at(hash % bios.size());
            
            // Get user avatar path
            QString avatarPath = getAvatarPathForUser(username);
            
            profilePage->setUserInfo(username, displayName, bio, avatarPath);
        }
    });
    
    // Friends page playback request (same as Profile)
    QObject::connect(friendsPage, &FriendsPage::playVideoRequested, &window, [&, homeNavButton](int index) {
        if (homeNavButton && !homeNavButton->isChecked()) {
            homeNavButton->setChecked(true);
        } else {
            stackedPages->setCurrentIndex(0);
        }
        playVideoAt(index);
        // voice narration
        NarrationManager::instance().narrate(
            QString::fromUtf8("播放视频 %1").arg(index + 1),
            QString("Playing video %1").arg(index + 1)
        );
    });

    QAction *accessibilityAction = nullptr;
    // Keyboard shortcut support - needs to be defined before use
    QVector<QShortcut *> keyboardShortcuts;
    auto rebuildKeyboardShortcuts = [&, volumeSlider]() {
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
        if (playPauseButton) {
            addShortcut(QKeySequence(Qt::Key_Space), [playPauseButton]() {
                playPauseButton->click();
            });
        }
        if (muteButton) {
            addShortcut(QKeySequence(Qt::Key_M), [muteButton]() {
                muteButton->click();
            });
        }
        if (prevMomentButton) {
            addShortcut(QKeySequence(Qt::ALT | Qt::Key_Left), [prevMomentButton]() {
                prevMomentButton->click();
            });
        }
        if (nextMomentButton) {
            addShortcut(QKeySequence(Qt::ALT | Qt::Key_Right), [nextMomentButton]() {
                nextMomentButton->click();
            });
        }
        if (player) {
            // Jump pass 5 seconds
            addShortcut(QKeySequence(Qt::Key_Right), [player]() {
                const qint64 duration = std::max<qint64>(0, player->duration());
                const qint64 nextPos = player->position() + 5000;
                player->setPosition(std::min(nextPos, duration));
            });
            // back 5 seconds
            addShortcut(QKeySequence(Qt::Key_Left), [player]() {
                const qint64 nextPos = player->position() - 5000;
                player->setPosition(std::max<qint64>(0, nextPos));
            });
            // adjust volumes
            addShortcut(QKeySequence(Qt::Key_Up), [player, volumeSlider]() {
                const int currentVolume = player->volume();
                const int newVolume = qMin(100, currentVolume + 5);
                player->setVolume(newVolume);
                // Synchronize volume bar
                if (volumeSlider) {
                    volumeSlider->blockSignals(true);
                    volumeSlider->setValue(newVolume);
                    volumeSlider->blockSignals(false);
                }
            });
            addShortcut(QKeySequence(Qt::Key_Down), [player, volumeSlider]() {
                const int currentVolume = player->volume();
                const int newVolume = qMax(0, currentVolume - 5);
                player->setVolume(newVolume);
                // Synchronize volume bar
                if (volumeSlider) {
                    volumeSlider->blockSignals(true);
                    volumeSlider->setValue(newVolume);
                    volumeSlider->blockSignals(false);
                }
            });
            // Use the [ / ] / 0 keys to control playback speed: 0.5x, 1.0x, 1.5x, 2.0 four levels
            addShortcut(QKeySequence(Qt::Key_BracketRight), [&accessibilityPrefs, player]() {
                static const double kRates[] = {0.5, 1.0, 1.5, 2.0};
                double rate = accessibilityPrefs.playbackRate;
                int idx = 0;
                for (int i = 0; i < 4; ++i) {
                    if (std::fabs(rate - kRates[i]) < 0.01) {
                        idx = i;
                        break;
                    }
                }
                if (idx < 3) {
                    idx++;
                }
                accessibilityPrefs.playbackRate = kRates[idx];
                player->setPlaybackRate(kRates[idx]);
            });
            addShortcut(QKeySequence(Qt::Key_BracketLeft), [&accessibilityPrefs, player]() {
                static const double kRates[] = {0.5, 1.0, 1.5, 2.0};
                double rate = accessibilityPrefs.playbackRate;
                int idx = 0;
                for (int i = 0; i < 4; ++i) {
                    if (std::fabs(rate - kRates[i]) < 0.01) {
                        idx = i;
                        break;
                    }
                }
                if (idx > 0) {
                    idx--;
                }
                accessibilityPrefs.playbackRate = kRates[idx];
                player->setPlaybackRate(kRates[idx]);
            });
            addShortcut(QKeySequence(Qt::Key_0), [&accessibilityPrefs, player]() {
                accessibilityPrefs.playbackRate = 1.0;
                player->setPlaybackRate(1.0);
            });
        }

        // Additional keyboard navigation (added on top of existing shortcuts)
        // Ctrl + Left / Up: Next video
        if (nextMomentButton) {
            addShortcut(QKeySequence(Qt::CTRL | Qt::Key_Left), [&, playVideoAt]() {
                playVideoAt(currentVideoIndex + 1);
            });
            addShortcut(QKeySequence(Qt::CTRL | Qt::Key_Up), [&, playVideoAt]() {
                playVideoAt(currentVideoIndex + 1);
            });
        }
        // Ctrl + Down: Previous video
        if (prevMomentButton) {
            addShortcut(QKeySequence(Qt::CTRL | Qt::Key_Down), [&, playVideoAt]() {
                playVideoAt(currentVideoIndex - 1);
            });
        }
        // Ctrl + Right: Go to Profile page
        if (profileNavButton) {
            addShortcut(QKeySequence(Qt::CTRL | Qt::Key_Right), [profileNavButton]() {
                profileNavButton->setChecked(true);
            });
        }
    };

    if (settingsButton) {
        auto *settingsMenu = new QMenu(settingsButton);
        accessibilityAction = settingsMenu->addAction(accessibilityPrefs.interfaceLanguage == AppLanguage::English
                                                          ? QStringLiteral("Accessibility Center")
                                                          : QString::fromUtf8("无障碍中心"));
        settingsMenu->addSeparator();
        QAction *dayModeAction = settingsMenu->addAction(QString());
        QAction *nightModeAction = settingsMenu->addAction(QString());

        QObject::connect(dayModeAction, &QAction::triggered, [applyTheme]() mutable {
            applyTheme(false);
        });
        QObject::connect(nightModeAction, &QAction::triggered, [applyTheme]() mutable {
            applyTheme(true);
        });

        // The interface language feature has been merged into AccessibilitySettingsDialog and is no longer duplicated in the Settings menu
        auto updateMenuLanguage = [settingsButton, settingsMenu, dayModeAction, nightModeAction, accessibilityAction, &accessibilityPrefs]() {
            const bool isChinese = accessibilityPrefs.interfaceLanguage == AppLanguage::Chinese;
            settingsButton->setText(isChinese ? QString::fromUtf8("设置") : "Settings");
            if (accessibilityAction) {
                accessibilityAction->setText(isChinese
                                                  ? QString::fromUtf8("无障碍中心")
                                                  : QStringLiteral("Accessibility Center"));
            }
            dayModeAction->setText(isChinese ? QString::fromUtf8("日间模式") : "Day mode");
            nightModeAction->setText(isChinese ? QString::fromUtf8("夜间模式") : "Night mode");
        };

        updateMenuLanguage();

        QObject::connect(settingsButton, &QPushButton::clicked, [settingsMenu, settingsButton]() {
            settingsMenu->exec(settingsButton->mapToGlobal(QPoint(settingsButton->width(), settingsButton->height())));
        });

        if (accessibilityAction) {
            QObject::connect(accessibilityAction, &QAction::triggered, [&]() {
                AccessibilitySettingsDialog dialog(accessibilityPrefs, subtitleLanguages,
                                                   accessibilityPrefs.interfaceLanguage, player, &window);
                if (dialog.exec() != QDialog::Accepted) {
                    return;
                }
                accessibilityPrefs = dialog.preferences();
                
                // Update LanguageManager so that RecordPage and FriendsPage will respond automatically
                LanguageManager::instance().setLanguage(accessibilityPrefs.interfaceLanguage);
                
                // Apply new accessibility settings
                subtitleController->setLanguage(accessibilityPrefs.subtitleLanguage);
                subtitleController->setEnabled(accessibilityPrefs.subtitlesEnabled);
                if (player) {
                    player->setPlaybackRate(accessibilityPrefs.playbackRate);
                }
                // Apply video display parameters (brightness, contrast, saturation)
                if (videoWidget) {
                    qreal brightness = (accessibilityPrefs.brightness - 1.0) * 100.0;
                    qreal contrast = (accessibilityPrefs.contrast - 1.0) * 100.0;
                    qreal saturation = (accessibilityPrefs.saturation - 1.0) * 100.0;
                    videoWidget->setBrightness(brightness);
                    videoWidget->setContrast(contrast);
                    videoWidget->setSaturation(saturation);
                }
                // Update voice broadcast settingss
                NarrationManager::instance().setEnabled(accessibilityPrefs.narrationEnabled);
                NarrationManager::instance().setLanguage(accessibilityPrefs.interfaceLanguage);
                
                // Update the language for all pages
                applyHomeLanguage();
                applyNavLanguage();
                if (profilePage) {
                    profilePage->setLanguage(accessibilityPrefs.interfaceLanguage);
                    profilePage->setHighContrastMode(accessibilityPrefs.colorBlindPaletteEnabled);
                }
                if (chatPage) {
                    chatPage->setLanguage(accessibilityPrefs.interfaceLanguage);
                    chatPage->setHighContrastMode(accessibilityPrefs.colorBlindPaletteEnabled);
                }
                updateMenuLanguage();
                
                // Reapply theme to reflect colorblind mode
                applyTheme(nightMode);
                // Rebuild keyboard shortcuts
                rebuildKeyboardShortcuts();
                // Update subtitle availability
                if (!videos.empty()) {
                    subtitleController->setActiveVideo(videos.at(currentVideoIndex).url->toLocalFile());
                }
                narrate(settingsButton->text());
            });
        }
    }
    
    rebuildKeyboardShortcuts();

    // showtime!
    window.show();

    // wait for the app to terminate
    return app.exec();
}
#endif // QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)

// MOC handling: For Q_OBJECT classes defined in .cpp files (SubtitleController and AccessibilitySettingsDialog),
// the .moc file needs to be included at the end of the file so that MOC can correctly generate the meta-object code
#include "tomeo.moc"










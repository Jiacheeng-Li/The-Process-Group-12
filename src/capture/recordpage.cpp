#include "recordpage.h"
#include <QResizeEvent>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QListWidget>
#include <QDialog>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include "popuppanel.h"
#include "../shared/language_manager.h"
#include "../shared/narration_manager.h"

RecordPage::RecordPage(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("recordPage");
    setStyleSheet(
        "QWidget#recordPage {"
        "  background-color: #00040d;"
        "  border: 3px solid #3A7DFF;"
        "  border-radius: 8px;"
        "}"
    );
      // Drop fixed minimum sizes in favor of a responsive layout

    phoneFrame = new QWidget(this);
    phoneFrame->setStyleSheet(
        "background-color: #f0f0f0;"
        "border: 4px solid #222;"
        "border-radius: 20px;"
        );

    backCamera = new QWidget(phoneFrame);
    backCamera->setStyleSheet("background-color: black;");

    frontCamera = new QWidget(backCamera);
    frontCamera->setStyleSheet(
        "background-color: #999;"
        "border: 2px solid white;"
        "border-radius: 8px;"
        );
    frontCamera->setCursor(Qt::PointingHandCursor);
    frontCamera->installEventFilter(this);

    recordButton = new QPushButton(backCamera);
    recordButton->setStyleSheet(
        "background-color: red;"
        "border-radius: 30px;"
        "border: 3px solid #6CADFF;"
        );
    recordButton->setFixedSize(60, 60);
    connect(recordButton, &QPushButton::clicked,
            this, &RecordPage::onRecordButtonClicked);

    pauseButton = new QPushButton(backCamera);
    pauseButton->setIcon(QIcon(":/icons/icons/play.svg"));  // ▶
    pauseButton->setIconSize(QSize(40,40));
    pauseButton->setStyleSheet(
        "background: rgba(255,255,255,230);"
        "border-radius: 22px;"
        "border: 2px solid #6CADFF;"
    );
    pauseButton->setFixedSize(44,44);
    connect(pauseButton, &QPushButton::clicked, this, &RecordPage::onPauseButtonClicked);

    switchButton = new QPushButton(backCamera);
    switchButton->setStyleSheet(
        "background-color: rgba(255,255,255,220);"
        "border-radius: 18px;"
        "border: 2px solid #6CADFF;"
        );
    switchButton->setFixedSize(36, 36);
    switchButton->setIcon(QIcon(":/icons/icons/switch_camera.svg"));
    switchButton->setIconSize(QSize(36, 36));

    connect(switchButton, &QPushButton::clicked,
            this, &RecordPage::onSwitchButtonClicked);

    connect(&pulseTimer, &QTimer::timeout, [this]() {
        pulseState = 1 - pulseState;
        recordButton->setStyleSheet(
            QString("background-color:%1;border-radius:30px;border:3px solid #6CADFF;")
                .arg(pulseState ? "#d80000" : "red")
            );
    });
    pulseTimer.setInterval(500);

    // Draft button
    draftButton = new QPushButton(this);
    draftButton->setStyleSheet(
        "background-color: rgba(255,255,255,220);"
        "border-radius: 18px;"
        "border: 2px solid #6CADFF;"
        "color: #333;"
        "font-weight: bold;"
        "padding: 8px 16px;"
        );
    draftButton->setFixedHeight(40);
    connect(draftButton, &QPushButton::clicked, this, &RecordPage::onDraftButtonClicked);

    auto &langMgr = LanguageManager::instance();
    applyTranslations(langMgr.currentLanguage());
    connect(&langMgr, &LanguageManager::languageChanged, this, &RecordPage::applyTranslations);
}
void RecordPage::applyTranslations(AppLanguage lang)
{
    if (!draftButton) {
        return;
    }
    const auto pick = [&](const QString &zh, const QString &en) {
        return lang == AppLanguage::Chinese ? zh : en;
    };
    draftButton->setText(pick("草稿箱", "Drafts"));
}


void RecordPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    const int W = width();
    const int H = height();

    // Control margins so fullscreen views stay full while keeping subtle padding
    const int horizontalMargin = qMax(24, W / 25);
    const int verticalMargin = qMax(32, H / 20);
    const double aspect = 9.0 / 16.0;

    int maxFrameW = qMax(280, W - horizontalMargin * 2);
    int maxFrameH = qMax(360, H - verticalMargin * 2);

    int frameW = maxFrameW;
    int frameH = static_cast<int>(frameW / aspect);
    if (frameH > maxFrameH) {
        frameH = maxFrameH;
        frameW = static_cast<int>(frameH * aspect);
    }

    // Center the frame, slightly offset upward to leave space for navigation
    int frameX = (W - frameW) / 2;
    int frameY = qMax(12, (H - frameH) / 2 - 24);
    phoneFrame->setGeometry(frameX, frameY, frameW, frameH);

    int margin = qMax(10, frameW / 40);
    backCamera->setGeometry(margin, margin, frameW - 2 * margin, frameH - 2 * margin);

    // Responsive sizing for the front camera overlay
    int fw = qMax(80, (frameW - 2 * margin) / 4);  // Minimum 80px, capped at one quarter of frame width
    int fh = static_cast<int>(fw * 4.0 / 3.0);  // Keep a 4:3 ratio
    int maxFh = (frameH - 2 * margin) / 3;  // Cap height at one-third of the frame
    if (fh > maxFh) {
        fh = maxFh;
        fw = static_cast<int>(fh * 3.0 / 4.0);
    }
    frontCamera->setGeometry(16, 16, fw, fh);

    // Record button centered near the bottom
    int rbSize = recordButton->width();
    int rbX = (frameW - 2 * margin - rbSize) / 2;
    int rbY = frameH - 2 * margin - rbSize - 16;
    recordButton->move(rbX, rbY);

    // Switch button anchored bottom-right
    int sbSize = switchButton->width();
    switchButton->move(frameW - 2 * margin - sbSize - 16, frameH - 2 * margin - sbSize - 16);

    // Pause button sits to the left of the record button
    pauseButton->move(rbX - sbSize - 16, rbY + (rbSize - pauseButton->height()) / 2);

    // Draft button hugs the lower-left of the black camera area
    int draftX = frameX + margin + 16;
    int draftY = frameY + frameH - margin - draftButton->height() - 16;
    draftButton->move(draftX, draftY);
}

void RecordPage::onRecordButtonClicked()
{
    if (!isRecording)
    {
        isRecording = true;
        pulseTimer.start();
        // 语音播报
        NarrationManager::instance().narrate(
            QString::fromUtf8("开始录制"),
            "Recording started"
        );
    }
    else
    {
        isRecording = false;
        pulseTimer.stop();
        // 语音播报
        NarrationManager::instance().narrate(
            QString::fromUtf8("停止录制"),
            "Recording stopped"
        );
        recordButton->setStyleSheet(
            "background-color: red;"
            "border-radius: 30px;"
            "border: 3px solid #6CADFF;"
            );
        emit recordingFinished();    
    }
}

void RecordPage::onSwitchButtonClicked()
{
    backIsBlack = !backIsBlack;

    if (backIsBlack) {
        backCamera->setStyleSheet("background-color: black;");
        frontCamera->setStyleSheet(
            "background-color:#999; border:2px solid white; border-radius:8px;"
            );
        // 语音播报
        NarrationManager::instance().narrate(
            QString::fromUtf8("切换到后置摄像头"),
            "Switched to back camera"
            );
    } else {
        backCamera->setStyleSheet("background-color:#999;");
        // 语音播报
        NarrationManager::instance().narrate(
            QString::fromUtf8("切换到前置摄像头"),
            "Switched to front camera"
        );
        frontCamera->setStyleSheet(
            "background-color:black; border:2px solid white; border-radius:8px;"
            );
    }
}

void RecordPage::onPauseButtonClicked()
{
    if (!isRecording) return;
    isPaused = !isPaused;

    if (isPaused) {
        pulseTimer.stop();
        recordButton->setStyleSheet("background:red;border-radius:30px;border:3px solid #6CADFF;");
        pauseButton->setIcon(QIcon(":/icons/icons/pause.svg"));   // Show pause icon while paused
        // 语音播报
        NarrationManager::instance().narrate(
            QString::fromUtf8("暂停录制"),
            "Recording paused"
        );
    } else {
        pulseTimer.start();
        // 语音播报
        NarrationManager::instance().narrate(
            QString::fromUtf8("继续录制"),
            "Recording resumed"
        );
        pauseButton->setIcon(QIcon(":/icons/icons/play.svg"));   // Revert to play icon when resuming
    }
}

bool RecordPage::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == frontCamera && event->type() == QEvent::MouseButtonPress) {
        onSwitchButtonClicked();
        return true;
    }
    return QWidget::eventFilter(watched, event);
}

// Simplified in-memory draft list; a real app would persist this
static QStringList draftList;

void RecordPage::onDraftButtonClicked()
{
    auto &langMgr = LanguageManager::instance();
    const auto pick = [&](const QString &zh, const QString &en) {
        return langMgr.pick(zh, en);
    };

    // 语音播报
    NarrationManager::instance().narrate(
        QString::fromUtf8("打开草稿箱"),
        "Open drafts"
    );

    if (draftList.isEmpty()) {
        QMessageBox::information(this,
                                 pick("草稿箱", "Drafts"),
                                 pick("草稿箱为空", "No drafts available yet."));
        return;
    }

    // 创建草稿选择对话框
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle(pick("选择草稿", "Select a draft"));
    dialog->setModal(true);
    dialog->setMinimumSize(400, 300);

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
    
    QLabel *titleLabel = new QLabel(pick("请选择要编辑的草稿：", "Choose a draft to resume:"), dialog);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; padding: 10px;");
    mainLayout->addWidget(titleLabel);

    QListWidget *listWidget = new QListWidget(dialog);
    listWidget->setStyleSheet(
        "QListWidget {"
        "  background: white;"
        "  border: 2px solid #ddd;"
        "  border-radius: 8px;"
        "  padding: 5px;"
        "}"
        "QListWidget::item {"
        "  padding: 10px;"
        "  border-bottom: 1px solid #eee;"
        "}"
        "QListWidget::item:hover {"
        "  background: #f0f0f0;"
        "}"
        "QListWidget::item:selected {"
        "  background: #e3f2fd;"
        "}"
        );
    
    for (int i = 0; i < draftList.size(); ++i) {
        QString preview = draftList[i];
        if (preview.length() > 50) {
            preview = preview.left(50) + "...";
        }
        listWidget->addItem(QString("%1 %2: %3")
                                .arg(pick("草稿", "Draft"))
                                .arg(i + 1)
                                .arg(preview));
    }

    mainLayout->addWidget(listWidget);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *selectBtn = new QPushButton(pick("选择", "Select"), dialog);
    QPushButton *deleteBtn = new QPushButton(pick("删除", "Delete"), dialog);
    QPushButton *cancelBtn = new QPushButton(pick("取消", "Cancel"), dialog);

    selectBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #2f8dff;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 8px 20px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #4ca2ff; }"
        );
    deleteBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #ff6b6b;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 8px 20px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #ff8787; }"
        );
    cancelBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #ccc;"
        "  color: #333;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 8px 20px;"
        "}"
        "QPushButton:hover { background-color: #ddd; }"
        );

    buttonLayout->addWidget(selectBtn);
    buttonLayout->addWidget(deleteBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelBtn);
    mainLayout->addLayout(buttonLayout);

    connect(selectBtn, &QPushButton::clicked, [=]() {
        int index = listWidget->currentRow();
        if (index >= 0 && index < draftList.size()) {
            emit draftSelected(draftList[index]);
            dialog->accept();
        }
    });

    connect(deleteBtn, &QPushButton::clicked, [=]() {
        int index = listWidget->currentRow();
        if (index >= 0 && index < draftList.size()) {
            draftList.removeAt(index);
            listWidget->takeItem(index);
            if (draftList.isEmpty()) {
                dialog->accept();
            }
        }
    });

    connect(cancelBtn, &QPushButton::clicked, dialog, &QDialog::reject);

    dialog->exec();
    delete dialog;
}

// Static helper to let PublishPage push drafts into the list
void RecordPage::addDraft(const QString &draftText)
{
    if (!draftText.trimmed().isEmpty()) {
        draftList.append(draftText);
    }
}


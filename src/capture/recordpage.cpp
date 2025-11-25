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

RecordPage::RecordPage(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("recordPage");
    setStyleSheet(
        "QWidget#recordPage { background-color: #00040d; }"
    );
    // 去掉固定最小尺寸，改用响应式布局

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
        "border: 3px solid white;"
        );
    recordButton->setFixedSize(60, 60);
    connect(recordButton, &QPushButton::clicked,
            this, &RecordPage::onRecordButtonClicked);

    pauseButton = new QPushButton(backCamera);
    pauseButton->setIcon(QIcon(":/icons/icons/play.svg"));  // ▶
    pauseButton->setIconSize(QSize(40,40));
    pauseButton->setStyleSheet("background:rgba(255,255,255,230);border-radius:22px;border:2px solid #444;");
    pauseButton->setFixedSize(44,44);
    connect(pauseButton, &QPushButton::clicked, this, &RecordPage::onPauseButtonClicked);

    switchButton = new QPushButton(backCamera);
    switchButton->setStyleSheet(
        "background-color: rgba(255,255,255,220);"
        "border-radius: 18px;"
        "border: 2px solid #444;"
        );
    switchButton->setFixedSize(36, 36);
    switchButton->setIcon(QIcon(":/icons/icons/switch_camera.svg"));
    switchButton->setIconSize(QSize(36, 36));

    connect(switchButton, &QPushButton::clicked,
            this, &RecordPage::onSwitchButtonClicked);

    connect(&pulseTimer, &QTimer::timeout, [this]() {
        pulseState = 1 - pulseState;
        recordButton->setStyleSheet(
            QString("background-color:%1;border-radius:30px;border:3px solid white;")
                .arg(pulseState ? "#d80000" : "red")
            );
    });
    pulseTimer.setInterval(500);

    // 草稿箱按钮
    draftButton = new QPushButton(this);
    draftButton->setText("草稿箱");
    draftButton->setStyleSheet(
        "background-color: rgba(255,255,255,220);"
        "border-radius: 18px;"
        "border: 2px solid #444;"
        "color: #333;"
        "font-weight: bold;"
        "padding: 8px 16px;"
        );
    draftButton->setFixedHeight(40);
    connect(draftButton, &QPushButton::clicked, this, &RecordPage::onDraftButtonClicked);
}

void RecordPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    int W = width();
    int H = height();

    // 确保有足够的最小尺寸
    if (W < 420) W = 420;
    if (H < 600) H = 600;

    // 计算手机框架大小，保持响应式
    int frameH = static_cast<int>(H * 0.75);  // 使用窗口高度的75%
    double aspect = 9.0 / 16.0;
    int frameW = static_cast<int>(frameH * aspect);

    // 限制最大宽度为窗口的85%，确保有边距
    int maxWidth = static_cast<int>(W * 0.85);
    if (frameW > maxWidth) {
        frameW = maxWidth;
        frameH = static_cast<int>(frameW / aspect);
    }

    // 限制最小尺寸，确保按钮可见
    int minFrameW = 280;
    int minFrameH = static_cast<int>(minFrameW / aspect);
    if (frameW < minFrameW) {
        frameW = minFrameW;
        frameH = minFrameH;
    }

    // 居中显示
    int frameX = (W - frameW) / 2;
    int frameY = qMax(12, (H - frameH) / 2 - 40);  // 稍微偏上，为导航栏留空间
    phoneFrame->setGeometry(frameX, frameY, frameW, frameH);

    int margin = 10;
    backCamera->setGeometry(margin, margin, frameW - 2 * margin, frameH - 2 * margin);

    // 前置摄像头大小，响应式调整
    int fw = qMax(80, (frameW - 2 * margin) / 4);  // 最小80px，最大为框架宽度的1/4
    int fh = static_cast<int>(fw * 4.0 / 3.0);  // 保持4:3比例
    int maxFh = (frameH - 2 * margin) / 3;  // 最大高度不超过框架高度的1/3
    if (fh > maxFh) {
        fh = maxFh;
        fw = static_cast<int>(fh * 3.0 / 4.0);
    }
    frontCamera->setGeometry(16, 16, fw, fh);

    // 录制按钮居中底部
    int rbSize = recordButton->width();
    int rbX = (frameW - 2 * margin - rbSize) / 2;
    int rbY = frameH - 2 * margin - rbSize - 16;
    recordButton->move(rbX, rbY);

    // 切换按钮右下角
    int sbSize = switchButton->width();
    switchButton->move(frameW - 2 * margin - sbSize - 16, frameH - 2 * margin - sbSize - 16);

    // 暂停按钮在录制按钮左侧
    pauseButton->move(rbX - sbSize - 16, rbY + (rbSize - pauseButton->height()) / 2);

    // 草稿箱按钮在左上角
    draftButton->move(frameX + 16, frameY + 16);
}

void RecordPage::onRecordButtonClicked()
{
    if (!isRecording)
    {
        isRecording = true;
        pulseTimer.start();
    }
    else
    {
        isRecording = false;
        pulseTimer.stop();
        recordButton->setStyleSheet(
            "background-color: red;"
            "border-radius: 30px;"
            "border: 3px solid white;"
            );
        emit recordingFinished();    // 🚀 进入发布页
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
    } else {
        backCamera->setStyleSheet("background-color:#999;");
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
        recordButton->setStyleSheet("background:red;border-radius:30px;border:3px solid white;");
        pauseButton->setIcon(QIcon(":/icons/icons/pause.svg"));   // || 恢复键
    } else {
        pulseTimer.start();
        pauseButton->setIcon(QIcon(":/icons/icons/play.svg"));   // ▶ 暂停键
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

// 静态草稿列表（简单实现，实际应该持久化存储）
static QStringList draftList;

void RecordPage::onDraftButtonClicked()
{
    if (draftList.isEmpty()) {
        QMessageBox::information(this, "草稿箱", "草稿箱为空");
        return;
    }

    // 创建草稿选择对话框
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("选择草稿");
    dialog->setModal(true);
    dialog->setMinimumSize(400, 300);

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
    
    QLabel *titleLabel = new QLabel("请选择要编辑的草稿：", dialog);
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
        listWidget->addItem(QString("草稿 %1: %2").arg(i + 1).arg(preview));
    }

    mainLayout->addWidget(listWidget);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *selectBtn = new QPushButton("选择", dialog);
    QPushButton *deleteBtn = new QPushButton("删除", dialog);
    QPushButton *cancelBtn = new QPushButton("取消", dialog);

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

// 静态函数：添加草稿（供 PublishPage 调用）
void RecordPage::addDraft(const QString &draftText)
{
    if (!draftText.trimmed().isEmpty()) {
        draftList.append(draftText);
    }
}


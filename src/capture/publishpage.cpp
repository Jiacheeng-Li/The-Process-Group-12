#include "publishpage.h"
#include <QResizeEvent>
#include <QMessageBox>
#include <QDebug>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <QRadioButton>      // <<< ★ 新增的头文件（解决你的报错）
#include <QRegExp>
#include "recordpage.h"

PublishPage::PublishPage(QWidget *parent)
    : QWidget(parent)
{

    inputBar = new QLineEdit(this);
    inputBar->setPlaceholderText("Write something...");
    inputBar->setStyleSheet("background:white;border-radius:6px;padding:6px;font-size:18px;");
    connect(inputBar, &QLineEdit::textChanged, this, &PublishPage::onUserTyped);

    bar5 = new QPushButton(this);
    bar6 = new QPushButton(this);
    bar7 = new QPushButton(this);
    bar5->setIcon(QIcon(":/icons/icons/volume.svg"));
    bar6->setIcon(QIcon(":/icons/icons/close.svg"));
    bar7->setIcon(QIcon(":/icons/icons/share.svg"));
    connect(bar6, &QPushButton::clicked, [this](){
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("取消发布");
        msgBox.setText("请选择：");
        QPushButton *saveDraft = msgBox.addButton("保存到草稿", QMessageBox::ActionRole);
        QPushButton *goBack    = msgBox.addButton("跳回录制页", QMessageBox::ActionRole);
        QPushButton *cancelBtn = msgBox.addButton("取消", QMessageBox::RejectRole);

        msgBox.exec();

        if (msgBox.clickedButton() == saveDraft) {
            draftBuffer = inputBar->text();
            // 保存草稿到草稿箱
            RecordPage::addDraft(draftBuffer);
            QMessageBox::information(this,"草稿","内容已保存到草稿！");
            emit backToRecord();  // 保存草稿后也返回录制页
            return;
        }
        if (msgBox.clickedButton() == goBack) {
            emit backToRecord();
            return;
        }
        // cancel → 什么都不做
    });


    phoneFrame = new QWidget(this);
    phoneFrame->setStyleSheet("background:#f0f0f0;border:4px solid #222;border-radius:20px;");
    backCamera = new QWidget(phoneFrame);
    backCamera->setStyleSheet("background:black;");
    frontCamera = new QWidget(backCamera);
    frontCamera->setStyleSheet("background:#999;border:2px solid white;border-radius:8px;");

    btn1 = new QPushButton(this);
    btn2 = new QPushButton(this);
    btn3 = new QPushButton(this);
    btn4 = new QPushButton(this);
    btn1->setIcon(QIcon(":/icons/icons/tag.svg"));
    btn2->setIcon(QIcon(":/icons/icons/mention.svg"));
    btn3->setIcon(QIcon(":/icons/icons/visibility.svg"));
    btn4->setIcon(QIcon(":/icons/icons/location_on.svg"));

    connect(btn1, &QPushButton::clicked, this, &PublishPage::onTagClicked);
    connect(btn2, &QPushButton::clicked, this, &PublishPage::onMentionClicked);
    connect(btn3, &QPushButton::clicked, this, &PublishPage::onPrivacyClicked);
    connect(btn4, &QPushButton::clicked, this, &PublishPage::onLocationClicked);
    connect(bar7, &QPushButton::clicked, this, &PublishPage::onShareClicked);
    connect(bar5, &QPushButton::clicked, this, &PublishPage::onVolumeClicked);

    sendButton = new QPushButton("Send >>>", this);
    sendButton->setFixedHeight(52);
    connect(sendButton, &QPushButton::clicked, [this](){
        QMessageBox::information(this,"Upload","Uploaded!");
        emit sendPressed("");
    });

    updateInputBar();
}

/* ------------ 输入区渲染核心 ------------ */
void PublishPage::updateInputBar()
{
    blockTextSignal = true;

    QString fixed;

    for (auto &tag : selectedTags)
        fixed += "#" + tag + "  ";

    for (auto &f : selectedFriends)
        fixed += "@" + f + "  ";

    QString finalText = fixed;
    if (!userText.isEmpty()) finalText += "\n" + userText;

    inputBar->setText(finalText);
    blockTextSignal = false;
}

void PublishPage::onUserTyped()
{
    if (blockTextSignal) return;

    QString txt = inputBar->text();
    QString fixed;

    for (auto &t : selectedTags) fixed += "#" + t + "  ";
    for (auto &f : selectedFriends) fixed += "@" + f + "  ";

    if (txt.startsWith(fixed))
        userText = txt.mid(fixed.length()).trimmed();
    else
        userText = txt;

    updateInputBar();
}

/* ------------ Tag 多选 ------------ */
void PublishPage::onTagClicked()
{
    if (!panelTag) {
        panelTag = new PopupPanel(this);
        panelTag->setTitle("选择标签");

        QWidget *box = new QWidget;
        QGridLayout *grid = new QGridLayout(box);
        grid->setContentsMargins(10, 10, 10, 10);
        grid->setHorizontalSpacing(16);
        grid->setVerticalSpacing(8);
        QStringList list = {"美食","旅行","冬天","校园","游戏","健身","打卡","学习","Vlog","情绪","随笔"};

        for (int i = 0; i < list.size(); ++i){
            const QString &t = list.at(i);
            QCheckBox *ck = new QCheckBox(t,box);
            ck->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
            connect(ck,&QCheckBox::stateChanged,[=](){
                if (ck->isChecked()) {
                    if (!selectedTags.contains(t)) selectedTags.append(t);
                } else {
                    selectedTags.removeAll(t);
                }
                updateInputBar();
            });
            int row = i / 2;
            int col = i % 2;
            grid->addWidget(ck, row, col);
        }

        panelTag->setContent(box);
    }
    panelTag->showPanel();
}

/* ------------ Mention 多选 ------------ */
void PublishPage::onMentionClicked()
{
    if (!panelMention) {
        panelMention = new PopupPanel(this);
        panelMention->setTitle("选择好友");

        QWidget *box = new QWidget;
        QVBoxLayout *lay = new QVBoxLayout(box);
        lay->setSpacing(6);
        lay->setContentsMargins(8, 8, 8, 8);
        QStringList list = {"Alice","Bob","Chris","David","Emily","Frank","Grace"};

        for (auto &f: list){
            QCheckBox *ck = new QCheckBox(f,box);
            connect(ck,&QCheckBox::stateChanged,[=](){
                if (ck->isChecked()) selectedFriends.append(f);
                else selectedFriends.removeAll(f);
                updateInputBar();
            });
            lay->addWidget(ck);
        }

        panelMention->setContent(box);
    }
    panelMention->showPanel();
}

/* ------------ 权限（单选） ------------ */
void PublishPage::onPrivacyClicked()
{
    if (!panelPrivacy) {
        panelPrivacy = new PopupPanel(this);
        panelPrivacy->setTitle("可见范围");

        QWidget *box = new QWidget;
        QVBoxLayout *lay = new QVBoxLayout(box);
        QStringList list = {"所有人可见","仅好友可见","仅自己可见"};

        for (auto &p: list){
            QRadioButton *rb = new QRadioButton(p,box);
            connect(rb,&QRadioButton::clicked,[=](){ privacySelection = p; });
            lay->addWidget(rb);
        }

        panelPrivacy->setContent(box);
    }
    panelPrivacy->showPanel();
}

/* ------------ 位置 ------------ */
void PublishPage::onLocationClicked()
{
    isLocationOn = !isLocationOn;
    btn4->setIcon(QIcon(isLocationOn ? ":/icons/icons/location_on.svg"
                                     : ":/icons/icons/location_off.svg"));
}

/* ------------ 分享 ------------ */
void PublishPage::onShareClicked()
{
    if (!panelShare) {
        panelShare = new PopupPanel(this);
        panelShare->setTitle("分享到");

        QWidget *box = new QWidget;
        QGridLayout *grid = new QGridLayout(box);
        grid->setContentsMargins(10, 10, 10, 10);
        grid->setHorizontalSpacing(16);
        grid->setVerticalSpacing(8);
        QStringList list = {"Instagram","Tinder","X (Twitter)","Telegram","微信","B站"};

        for (int i = 0; i < list.size(); ++i) {
            const QString &p = list.at(i);
            QLabel *label = new QLabel("• " + p, box);
            label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
            int row = i / 2;
            int col = i % 2;
            grid->addWidget(label, row, col);
        }

        QPushButton *ok = new QPushButton("确定转发");
        QPushButton *cancel = new QPushButton("取消");
        connect(cancel,&QPushButton::clicked,[=](){ panelShare->hidePanel(); });
        connect(ok,&QPushButton::clicked,[=](){
            QMessageBox::information(this,"分享","已同步分享到其他平台！");
            panelShare->hidePanel();
        });
        // 按钮放在最后一行，跨两列
        grid->addWidget(ok, (list.size() + 1) / 2, 0, 1, 2);
        grid->addWidget(cancel, (list.size() + 1) / 2 + 1, 0, 1, 2);

        panelShare->setContent(box);
    }
    panelShare->showPanel();
}

/* ------------ 音量 ------------ */
void PublishPage::onVolumeClicked()
{
    if (!panelVolume) {
        panelVolume = new PopupPanel(this);
        panelVolume->setTitle("音量");

        QWidget *box = new QWidget;
        QVBoxLayout *lay = new QVBoxLayout(box);
        QLabel *lab = new QLabel("调整音量");
        volumeSlider = new QSlider(Qt::Horizontal);
        volumeSlider->setRange(0,100);
        volumeSlider->setValue(75);
        lay->addWidget(lab);
        lay->addWidget(volumeSlider);

        panelVolume->setContent(box);
    }
    panelVolume->showPanel();
}

void PublishPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    const int W = width();
    const int H = height();
    const int outerMargin = qMax(32, W / 18);
    const int topMargin = 28;
    const int gap = 20;
    const int iconSize = 46;
    const int iconGap = 10;

    // 计算手机框尺寸（保持与 Record 页类似的响应方式）
    int maxFrameW = W - outerMargin * 2;
    int frameH = qBound(360, static_cast<int>(H * 0.62), H - 240);
    int frameW = qMin(maxFrameW, static_cast<int>(frameH * 9.0 / 16.0));
    frameH = static_cast<int>(frameW * 16.0 / 9.0);
    int frameLeft = (W - frameW) / 2;

    // 输入栏与图标按钮（同一行，左右对齐手机框）
    const int iconsTotalWidth = iconSize * 3 + iconGap * 2;
    int iconsStartX = frameLeft + frameW - iconsTotalWidth;
    iconsStartX = qMax(frameLeft + iconGap, iconsStartX);
    bar5->setGeometry(iconsStartX, topMargin, iconSize, iconSize);
    bar6->setGeometry(iconsStartX + iconSize + iconGap, topMargin, iconSize, iconSize);
    bar7->setGeometry(iconsStartX + 2 * (iconSize + iconGap), topMargin, iconSize, iconSize);

    int inputWidth = iconsStartX - iconGap - frameLeft;
    inputWidth = qMax(200, inputWidth);
    inputBar->setGeometry(frameLeft, topMargin, inputWidth, iconSize);

    int frameTop = topMargin + iconSize + gap;

    phoneFrame->setGeometry(frameLeft, frameTop, frameW, frameH);

    int frameMargin = 10;
    backCamera->setGeometry(frameMargin, frameMargin, frameW - frameMargin * 2, frameH - frameMargin * 2);
    frontCamera->setGeometry(
        frameMargin + 10,
        frameMargin + 10,
        qMax(80, static_cast<int>((frameW - frameMargin * 2) * 0.3)),
        qMax(60, static_cast<int>((frameH - frameMargin * 2) * 0.3))
        );

    int y = frameTop + frameH + gap;

    // 工具按钮（Tag 等）
    const int tagH = 46;
    const int tagSpacing = 12;
    int eachTagW = (frameW - tagSpacing * 3) / 4;
    eachTagW = qMax(88, eachTagW);

    btn1->setGeometry(frameLeft, y, eachTagW, tagH);
    btn2->setGeometry(frameLeft + (eachTagW + tagSpacing), y, eachTagW, tagH);
    btn3->setGeometry(frameLeft + 2 * (eachTagW + tagSpacing), y, eachTagW, tagH);
    btn4->setGeometry(frameLeft + 3 * (eachTagW + tagSpacing), y, eachTagW, tagH);

    y += tagH + gap;

    // 发送按钮
    sendButton->setGeometry(frameLeft, y, frameW, 54);
}

void PublishPage::loadDraft(const QString &draftText)
{
    // 清空当前内容
    selectedTags.clear();
    selectedFriends.clear();
    userText = "";
    draftBuffer = "";
    
    // 加载草稿文本
    blockTextSignal = true;
    inputBar->setText(draftText);
    blockTextSignal = false;
    
    // 解析草稿中的标签和好友（简单实现）
    QString text = draftText;
    // 提取标签 #tag
    QRegExp tagRegex("#(\\w+)");
    int pos = 0;
    while ((pos = tagRegex.indexIn(text, pos)) != -1) {
        QString tag = tagRegex.cap(1);
        if (!selectedTags.contains(tag)) {
            selectedTags.append(tag);
        }
        pos += tagRegex.matchedLength();
    }
    
    // 提取好友 @friend
    QRegExp friendRegex("@(\\w+)");
    pos = 0;
    while ((pos = friendRegex.indexIn(text, pos)) != -1) {
        QString friendName = friendRegex.cap(1);
        if (!selectedFriends.contains(friendName)) {
            selectedFriends.append(friendName);
        }
        pos += friendRegex.matchedLength();
    }
    
    // 更新输入栏显示
    updateInputBar();
}


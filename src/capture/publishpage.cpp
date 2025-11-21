#include "publishpage.h"
#include <QResizeEvent>
#include <QMessageBox>
#include <QDebug>
#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QRadioButton>      // <<< ★ 新增的头文件（解决你的报错）

PublishPage::PublishPage(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(1260,760);

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
    connect(bar6, &QPushButton::clicked, this, &PublishPage::backToRecord);

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
        QVBoxLayout *lay = new QVBoxLayout(box);
        QStringList list = {"美食","旅行","冬天","校园","游戏","健身","打卡","学习","Vlog","情绪","随笔"};

        for (auto &t: list){
            QCheckBox *ck = new QCheckBox(t,box);
            connect(ck,&QCheckBox::stateChanged,[=](){
                if (ck->isChecked()) {
                    if (!selectedTags.contains(t)) selectedTags.append(t);
                } else {
                    selectedTags.removeAll(t);
                }
                updateInputBar();
            });
            lay->addWidget(ck);
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
    btn4->setIcon(QIcon(isLocationOn ? ":/icons/location_on.svg"
                                     : ":/icons/location_off.svg"));
}

/* ------------ 分享 ------------ */
void PublishPage::onShareClicked()
{
    if (!panelShare) {
        panelShare = new PopupPanel(this);
        panelShare->setTitle("分享到");

        QWidget *box = new QWidget;
        QVBoxLayout *lay = new QVBoxLayout(box);
        QStringList list = {"Instagram","Tinder","X (Twitter)","Telegram","微信","B站"};

        for (auto &p: list)
            lay->addWidget(new QLabel("• " + p));

        QPushButton *ok = new QPushButton("确定转发");
        QPushButton *cancel = new QPushButton("取消");
        connect(cancel,&QPushButton::clicked,[=](){ panelShare->hidePanel(); });
        connect(ok,&QPushButton::clicked,[=](){
            QMessageBox::information(this,"分享","已同步分享到其他平台！");
            panelShare->hidePanel();
        });
        lay->addWidget(ok);
        lay->addWidget(cancel);

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

    int W = width();
    int H = height();

    const int topMargin = 20;
    const int gap       = 18;

    int maxContentW = 440;
    int totalW      = qMin(W - 40, maxContentW);
    if (totalW < 320)
        totalW = W - 40;

    int leftX = (W - totalW) / 2;

    int iconBtnSize = 44;
    int y1 = topMargin;
    int inputW = totalW - iconBtnSize * 3 - gap * 2;

    inputBar->setGeometry(leftX, y1, inputW, iconBtnSize);

    int firstIconX = leftX + inputW + gap;
    int iconGap = 8;
    bar5->setGeometry(firstIconX, y1, iconBtnSize, iconBtnSize);
    bar6->setGeometry(firstIconX + iconBtnSize + iconGap, y1, iconBtnSize, iconBtnSize);
    bar7->setGeometry(firstIconX + (iconBtnSize + iconGap) * 2, y1, iconBtnSize, iconBtnSize);

    int y2 = y1 + iconBtnSize + gap;
    int phoneH = static_cast<int>(H * 0.48);
    phoneFrame->setGeometry(leftX, y2, totalW, phoneH);

    backCamera->setGeometry(10, 10, totalW - 20, phoneH - 20);
    frontCamera->setGeometry(
        18, 18,
        static_cast<int>((totalW - 20) * 0.28),
        static_cast<int>((phoneH - 20) * 0.32)
        );

    int y3 = y2 + phoneH + gap;
    int tagH = 44;
    int spacing3 = 12;
    int tagW = (totalW - spacing3 * 3) / 4;

    int x3 = (W - totalW) / 2;
    btn1->setGeometry(x3, y3, tagW, tagH);
    btn2->setGeometry(x3 + (tagW + spacing3), y3, tagW, tagH);
    btn3->setGeometry(x3 + 2 * (tagW + spacing3), y3, tagW, tagH);
    btn4->setGeometry(x3 + 3 * (tagW + spacing3), y3, tagW, tagH);

    int y4 = y3 + tagH + gap;
    sendButton->setGeometry((W - totalW) / 2, y4, totalW, 52);
}


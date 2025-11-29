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
#include <QStringLiteral>
#include "recordpage.h"
#include "../shared/language_manager.h"

PublishPage::PublishPage(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("publishPage");
    setStyleSheet(
        "QWidget#publishPage {"
        "  background-color: #00040d;"
        "  border: 3px solid #FF4F70;"
        "  border-radius: 8px;"
        "}"
    );

    auto appendOption = [](QVector<LocalizedOption> &list,
                           const QString &key,
                           const QString &zh,
                           const QString &en) {
        LocalizedOption opt;
        opt.key = key;
        opt.zh = zh;
        opt.en = en;
        list.append(opt);
    };

    appendOption(tagOptions_, QStringLiteral("food"), QString::fromUtf8("美食"), QStringLiteral("Food"));
    appendOption(tagOptions_, QStringLiteral("travel"), QString::fromUtf8("旅行"), QStringLiteral("Travel"));
    appendOption(tagOptions_, QStringLiteral("winter"), QString::fromUtf8("冬天"), QStringLiteral("Winter"));
    appendOption(tagOptions_, QStringLiteral("campus"), QString::fromUtf8("校园"), QStringLiteral("Campus"));
    appendOption(tagOptions_, QStringLiteral("gaming"), QString::fromUtf8("游戏"), QStringLiteral("Gaming"));
    appendOption(tagOptions_, QStringLiteral("fitness"), QString::fromUtf8("健身"), QStringLiteral("Workout"));
    appendOption(tagOptions_, QStringLiteral("checkin"), QString::fromUtf8("打卡"), QStringLiteral("Daily check-in"));
    appendOption(tagOptions_, QStringLiteral("study"), QString::fromUtf8("学习"), QStringLiteral("Study"));
    appendOption(tagOptions_, QStringLiteral("vlog"), QString::fromUtf8("Vlog"), QStringLiteral("Vlog"));
    appendOption(tagOptions_, QStringLiteral("mood"), QString::fromUtf8("情绪"), QStringLiteral("Mood"));
    appendOption(tagOptions_, QStringLiteral("diary"), QString::fromUtf8("随笔"), QStringLiteral("Diary"));

    appendOption(privacyOptions_, QStringLiteral("all"), QString::fromUtf8("所有人可见"),
                 QStringLiteral("Visible to everyone"));
    appendOption(privacyOptions_, QStringLiteral("friends"), QString::fromUtf8("仅好友可见"),
                 QStringLiteral("Friends only"));
    appendOption(privacyOptions_, QStringLiteral("private"), QString::fromUtf8("仅自己可见"),
                 QStringLiteral("Only me"));

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
        auto &langMgr = LanguageManager::instance();
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(langMgr.pick(QString::fromUtf8("取消发布"),
                                           QStringLiteral("Cancel posting")));
        msgBox.setText(langMgr.pick(QString::fromUtf8("请选择："),
                                    QStringLiteral("Choose an option:")));
        QPushButton *saveDraft = msgBox.addButton(
            langMgr.pick(QString::fromUtf8("保存到草稿"),
                         QStringLiteral("Save to drafts")), QMessageBox::ActionRole);
        QPushButton *goBack = msgBox.addButton(
            langMgr.pick(QString::fromUtf8("跳回录制页"),
                         QStringLiteral("Return to record page")), QMessageBox::ActionRole);
        QPushButton *cancelBtn = msgBox.addButton(
            langMgr.pick(QString::fromUtf8("取消"),
                         QStringLiteral("Cancel")), QMessageBox::RejectRole);

        msgBox.exec();

        if (msgBox.clickedButton() == saveDraft) {
            draftBuffer = inputBar->text();
            // 保存草稿到草稿箱
            RecordPage::addDraft(draftBuffer);
            QMessageBox::information(
                this,
                langMgr.pick(QString::fromUtf8("草稿"),
                             QStringLiteral("Drafts")),
                langMgr.pick(QString::fromUtf8("内容已保存到草稿！"),
                             QStringLiteral("Saved to drafts!")));
            emit backToRecord();  // 保存草稿后也返回录制页
            return;
        }
        if (msgBox.clickedButton() == goBack) {
            emit backToRecord();
            return;
        }
    });


    phoneFrame = new QWidget(this);
    phoneFrame->setStyleSheet(
        "background: #0D0D0D;"
        "border: 4px solid #3A7DFF;"
        "border-radius: 20px;"
    );
    backCamera = new QWidget(phoneFrame);
    backCamera->setStyleSheet("background:black;");
    frontCamera = new QWidget(backCamera);
    frontCamera->setStyleSheet("background:#999;border:2px solid white;border-radius:8px;");

    btn1 = new QPushButton("Tag", this);
    btn2 = new QPushButton("Friends", this);
    btn3 = new QPushButton("Privacy", this);
    btn4 = new QPushButton("Location", this);
    btn1->setIcon(QIcon(":/icons/icons/tag.svg"));
    btn1->setIconSize(QSize(28, 28));
    btn2->setIcon(QIcon(":/icons/icons/mention.svg"));
    btn2->setIconSize(QSize(28, 28));
    btn3->setIcon(QIcon(":/icons/icons/visibility.svg"));
    btn3->setIconSize(QSize(28, 28));
    btn4->setIcon(QIcon(":/icons/icons/location_on.svg"));
    btn4->setIconSize(QSize(28, 28));
    
    // 美化按钮样式 - 图标+文字，类似share_to_friends的效果
    QString buttonStyle = 
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(108,173,255,0.25), stop:1 rgba(58,125,255,0.35));"
        "  border: 2px solid #6CADFF;"
        "  border-radius: 18px;"
        "  padding: 10px 16px;"
        "  color: white;"
        "  font-size: 16px;"
        "  font-weight: 600;"
        "  text-align: left;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(108,173,255,0.4), stop:1 rgba(58,125,255,0.5));"
        "  border-color: #3A7DFF;"
        "}"
        "QPushButton:pressed {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #6CADFF, stop:1 #3A7DFF);"
        "}";
    btn1->setStyleSheet(buttonStyle);
    btn2->setStyleSheet(buttonStyle);
    btn3->setStyleSheet(buttonStyle);
    btn4->setStyleSheet(buttonStyle);
    
    // 美化顶部按钮 - 使用混合配色
    QString topButtonStyle =
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(191,191,191,0.2), stop:1 rgba(108,173,255,0.3));"
        "  border: 2px solid #6CADFF;"
        "  border-radius: 6px;"
        "  padding: 6px;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(108,173,255,0.4), stop:1 rgba(58,125,255,0.5));"
        "  border-color: #3A7DFF;"
        "}";
    bar5->setStyleSheet(topButtonStyle);
    bar6->setStyleSheet(topButtonStyle);
    bar7->setStyleSheet(topButtonStyle);

    connect(btn1, &QPushButton::clicked, this, &PublishPage::onTagClicked);
    connect(btn2, &QPushButton::clicked, this, &PublishPage::onMentionClicked);
    connect(btn3, &QPushButton::clicked, this, &PublishPage::onPrivacyClicked);
    connect(btn4, &QPushButton::clicked, this, &PublishPage::onLocationClicked);
    connect(bar7, &QPushButton::clicked, this, &PublishPage::onShareClicked);
    connect(bar5, &QPushButton::clicked, this, &PublishPage::onVolumeClicked);

    sendButton = new QPushButton("Send >>>", this);
    sendButton->setFixedHeight(52);
    sendButton->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #FF4F70, stop:0.5 #6CADFF, stop:1 #3A7DFF);"
        "  color: white;"
        "  border: 2px solid #6CADFF;"
        "  border-radius: 8px;"
        "  font-weight: bold;"
        "  font-size: 16px;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ff6b8a, stop:0.5 #7db8ff, stop:1 #4a8dff);"
        "  border-color: #3A7DFF;"
        "}"
    );
    connect(sendButton, &QPushButton::clicked, [this](){
        QMessageBox::information(this,"Upload","Uploaded!");
        emit sendPressed("");
    });

    applyLanguage();
    QObject::connect(&LanguageManager::instance(), &LanguageManager::languageChanged,
                     this, [this](AppLanguage) { applyLanguage(); });
}

/* ------------ Input rendering core ------------ */
void PublishPage::updateInputBar()
{
    blockTextSignal = true;

    QString fixed;

    for (const auto &tagKey : selectedTags)
        fixed += "#" + tagDisplayText(tagKey) + "  ";

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

    for (const auto &tagKey : selectedTags) fixed += "#" + tagDisplayText(tagKey) + "  ";
    for (auto &f : selectedFriends) fixed += "@" + f + "  ";

    if (txt.startsWith(fixed))
        userText = txt.mid(fixed.length()).trimmed();
    else
        userText = txt;

    updateInputBar();
}

/* ------------ Tag picker (multi-select) ------------ */
void PublishPage::onTagClicked()
{
    if (!panelTag) {
        panelTag = new PopupPanel(this);
        panelTag->setTitle(LanguageManager::instance().pick(QString::fromUtf8("选择标签"),
                                                            QStringLiteral("Select tags")));

        QWidget *box = new QWidget;
        QGridLayout *grid = new QGridLayout(box);
        grid->setContentsMargins(10, 10, 10, 10);
        grid->setHorizontalSpacing(16);
        grid->setVerticalSpacing(24);  // 适中的行间距，保持清晰但不占用过多空间
        for (int i = 0; i < tagOptions_.size(); ++i){
            auto &opt = tagOptions_[i];
            QCheckBox *ck = new QCheckBox(opt.en,box);
            opt.button = ck;
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
            ck->setChecked(selectedTags.contains(opt.key));
            connect(ck,&QCheckBox::stateChanged,[=]() {
                if (ck->isChecked()) {
                    if (!selectedTags.contains(opt.key)) selectedTags.append(opt.key);
                } else {
                    selectedTags.removeAll(opt.key);
                }
                updateInputBar();
            });
            int row = i / 2;
            int col = i % 2;
            grid->addWidget(ck, row, col);
        }

        panelTag->setContent(box);
        applyLanguage();
    }
    panelTag->showPanel();
}

/* ------------ Mention picker (multi-select) ------------ */
void PublishPage::onMentionClicked()
{
    if (!panelMention) {
        panelMention = new PopupPanel(this);
        panelMention->setTitle(LanguageManager::instance().pick(QString::fromUtf8("选择好友"),
                                                                QStringLiteral("Select friends")));

        QWidget *box = new QWidget;
        QVBoxLayout *lay = new QVBoxLayout(box);
        lay->setSpacing(20);  // 适中的行间距，保持清晰但不占用过多空间
        lay->setContentsMargins(8, 8, 8, 8);
        QStringList list = {"Alice","Bob","Chris","David","Emily","Frank","Grace"};

        for (auto &f: list){
            QCheckBox *ck = new QCheckBox(f,box);
            ck->setStyleSheet(
                "QCheckBox {"
                "  color: white;"
                "  font-size: 16px;"
                "  font-weight: 600;"
                "  padding: 12px 16px;"
                "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(108,173,255,0.1), stop:1 rgba(58,125,255,0.15));"
                "  border: 1px solid #6CADFF;"
                "  border-radius: 8px;"
                "}"
                "QCheckBox::indicator {"
                "  width: 20px;"
                "  height: 20px;"
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
            connect(ck,&QCheckBox::stateChanged,[=](){
                if (ck->isChecked()) selectedFriends.append(f);
                else selectedFriends.removeAll(f);
                updateInputBar();
            });
            lay->addWidget(ck);
        }

        panelMention->setContent(box);
        applyLanguage();
    }
    panelMention->showPanel();
}

/* ------------ Visibility (single-select) ------------ */
void PublishPage::onPrivacyClicked()
{
    if (!panelPrivacy) {
        panelPrivacy = new PopupPanel(this);
        panelPrivacy->setTitle(LanguageManager::instance().pick(QString::fromUtf8("可见范围"),
                                                                QStringLiteral("Visibility")));

        QWidget *box = new QWidget;
        QVBoxLayout *lay = new QVBoxLayout(box);
        lay->setSpacing(20);  // 适中的行间距，保持清晰但不占用过多空间
        lay->setContentsMargins(8, 8, 8, 8);
        for (auto &opt : privacyOptions_){
            QRadioButton *rb = new QRadioButton(opt.en,box);
            opt.button = rb;
            rb->setStyleSheet(
                "QRadioButton {"
                "  color: white;"
                "  font-size: 16px;"
                "  font-weight: 600;"
                "  padding: 12px 16px;"
                "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(108,173,255,0.1), stop:1 rgba(58,125,255,0.15));"
                "  border: 1px solid #6CADFF;"
                "  border-radius: 8px;"
                "}"
                "QRadioButton::indicator {"
                "  width: 20px;"
                "  height: 20px;"
                "  border: 2px solid #6CADFF;"
                "  border-radius: 10px;"
                "  background: transparent;"
                "}"
                "QRadioButton::indicator:checked {"
                "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #6CADFF, stop:1 #FF4F70);"
                "  border-color: #FF4F70;"
                "}"
                "QRadioButton:hover {"
                "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(108,173,255,0.2), stop:1 rgba(255,79,112,0.2));"
                "  border-color: #FF4F70;"
                "}"
            );
            rb->setChecked(privacySelection == opt.key);
            connect(rb,&QRadioButton::clicked,[=](){ privacySelection = opt.key; });
            lay->addWidget(rb);
        }

        panelPrivacy->setContent(box);
        applyLanguage();
    }
    panelPrivacy->showPanel();
}

/* ------------ Location toggle ------------ */
void PublishPage::onLocationClicked()
{
    isLocationOn = !isLocationOn;
    btn4->setIcon(QIcon(isLocationOn ? ":/icons/icons/location_on.svg"
                                     : ":/icons/icons/location_off.svg"));
}

/* ------------ Share panel ------------ */
void PublishPage::onShareClicked()
{
    if (!panelShare) {
        panelShare = new PopupPanel(this);
        panelShare->setTitle(LanguageManager::instance().pick(QString::fromUtf8("分享到"),
                                                              QStringLiteral("Share to")));

        QWidget *box = new QWidget;
        QGridLayout *grid = new QGridLayout(box);
        grid->setContentsMargins(10, 10, 10, 10);
        grid->setHorizontalSpacing(16);
        grid->setVerticalSpacing(24);  // 适中的行间距，保持清晰但不占用过多空间
        QStringList list = {"Instagram","Tinder","X (Twitter)","Telegram","微信","B站"};

        for (int i = 0; i < list.size(); ++i) {
            const QString &p = list.at(i);
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
            int row = i / 2;
            int col = i % 2;
            grid->addWidget(ck, row, col);
        }

        QPushButton *ok = new QPushButton("确定转发");
        QPushButton *cancel = new QPushButton("取消");
        ok->setStyleSheet(
            "QPushButton {"
            "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #FF4F70, stop:0.5 #6CADFF, stop:1 #3A7DFF);"
            "  color: white;"
            "  border: 2px solid #6CADFF;"
            "  border-radius: 8px;"
            "  padding: 10px 20px;"
            "  font-weight: bold;"
            "  font-size: 14px;"
            "}"
            "QPushButton:hover {"
            "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ff6b8a, stop:0.5 #7db8ff, stop:1 #4a8dff);"
            "  border-color: #FF4F70;"
            "}"
        );
        cancel->setStyleSheet(
            "QPushButton {"
            "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(191,191,191,0.3), stop:1 rgba(108,173,255,0.2));"
            "  color: white;"
            "  border: 2px solid #BFBFBF;"
            "  border-radius: 8px;"
            "  padding: 10px 20px;"
            "  font-weight: bold;"
            "  font-size: 14px;"
            "}"
            "QPushButton:hover {"
            "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(191,191,191,0.5), stop:1 rgba(108,173,255,0.4));"
            "  border-color: #6CADFF;"
            "}"
        );
        connect(cancel,&QPushButton::clicked,[=](){ panelShare->hidePanel(); });
        connect(ok,&QPushButton::clicked,[=](){
            QMessageBox::information(
                this,
                LanguageManager::instance().pick(QString::fromUtf8("分享"),
                                                 QStringLiteral("Share")),
                LanguageManager::instance().pick(QString::fromUtf8("已同步分享到其他平台！"),
                                                 QStringLiteral("Shared to other platforms!")));
            panelShare->hidePanel();
        });
        // 按钮放在最后一行，跨两列
        grid->addWidget(ok, (list.size() + 1) / 2, 0, 1, 2);
        grid->addWidget(cancel, (list.size() + 1) / 2 + 1, 0, 1, 2);

        panelShare->setContent(box);
        applyLanguage();
    }
    panelShare->showPanel();
}

/* ------------ Volume panel ------------ */
void PublishPage::onVolumeClicked()
{
    if (!panelVolume) {
        panelVolume = new PopupPanel(this);
        panelVolume->setTitle(LanguageManager::instance().pick(QString::fromUtf8("音量"),
                                                               QStringLiteral("Volume")));

        QWidget *box = new QWidget;
        QVBoxLayout *lay = new QVBoxLayout(box);
        volumeLabel = new QLabel(QStringLiteral("Adjust volume"));
        volumeSlider = new QSlider(Qt::Horizontal);
        volumeSlider->setRange(0,100);
        volumeSlider->setValue(75);
        lay->addWidget(volumeLabel);
        lay->addWidget(volumeSlider);

        panelVolume->setContent(box);
        applyLanguage();
    }
    panelVolume->showPanel();
}

void PublishPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    const int W = width();
    const int H = height();
    const int outerMargin = qMax(32, W / 24);
    const int topMargin = qMax(28, H / 30);
    const int gap = qMax(18, H / 60);
    const int iconSize = 52;
    const int iconGap = 14;
    const int tagH = 64;
    const int bottomMargin = qMax(32, H / 18);

    const int frameTop = topMargin + iconSize + gap;
    const int reservedBelow = gap + tagH + gap + 54 + bottomMargin;
    int availableHeight = qMax(360, H - frameTop - reservedBelow);
    int maxFrameW = qMax(320, W - outerMargin * 2);
    int frameW = maxFrameW;
    int frameH = static_cast<int>(frameW * 16.0 / 9.0);
    if (frameH > availableHeight) {
        frameH = availableHeight;
        frameW = static_cast<int>(frameH * 9.0 / 16.0);
    }
    int frameLeft = (W - frameW) / 2;

    // Align input bar and quick-action icons along the phone frame
    const int iconsTotalWidth = iconSize * 3 + iconGap * 2;
    int iconsStartX = frameLeft + frameW - iconsTotalWidth;
    iconsStartX = qMax(frameLeft + iconGap, iconsStartX);
    bar5->setGeometry(iconsStartX, topMargin, iconSize, iconSize);
    bar6->setGeometry(iconsStartX + iconSize + iconGap, topMargin, iconSize, iconSize);
    bar7->setGeometry(iconsStartX + 2 * (iconSize + iconGap), topMargin, iconSize, iconSize);

    int inputWidth = iconsStartX - iconGap - frameLeft;
    inputWidth = qMax(200, inputWidth);
    inputBar->setGeometry(frameLeft, topMargin, inputWidth, iconSize);

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
    y = qMin(y, H - bottomMargin - tagH - gap - 54);

    // Utility buttons (tag/friend/privacy/loc) – 始终 4 个按钮在一行，严格对齐 frame 左右
    const int tagSpacing = 16;
    const int tagCols = 4;
    int eachTagW = (frameW - tagSpacing * (tagCols - 1)) / tagCols;
    eachTagW = qMax(110, eachTagW); // 稍微放小一点以适配窄屏

    int tagRowX = frameLeft;
    btn1->setGeometry(tagRowX, y, eachTagW, tagH);
    btn2->setGeometry(tagRowX + (eachTagW + tagSpacing), y, eachTagW, tagH);
    btn3->setGeometry(tagRowX + 2 * (eachTagW + tagSpacing), y, eachTagW, tagH);
    btn4->setGeometry(tagRowX + 3 * (eachTagW + tagSpacing), y, eachTagW, tagH);

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
    // Extract hashtags (#tag)
    QRegExp tagRegex("#(\\w+)");
    int pos = 0;
    while ((pos = tagRegex.indexIn(text, pos)) != -1) {
        QString tag = tagRegex.cap(1);
        const QString key = tagKeyForLabel(tag);
        if (!selectedTags.contains(key)) {
            selectedTags.append(key);
        }
        pos += tagRegex.matchedLength();
    }
    
    // Extract friend mentions (@friend)
    QRegExp friendRegex("@(\\w+)");
    pos = 0;
    while ((pos = friendRegex.indexIn(text, pos)) != -1) {
        QString friendName = friendRegex.cap(1);
        if (!selectedFriends.contains(friendName)) {
            selectedFriends.append(friendName);
        }
        pos += friendRegex.matchedLength();
    }
    
    // Refresh the composed input text with tags and mentions
    updateInputBar();
}

void PublishPage::applyLanguage()
{
    const auto pick = [&](const QString &zh, const QString &en) {
        return LanguageManager::instance().pick(zh, en);
    };

    if (panelTag) {
        panelTag->setTitle(pick(QString::fromUtf8("选择标签"), QStringLiteral("Select tags")));
    }
    for (auto &opt : tagOptions_) {
        if (opt.button) {
            opt.button->setText(pick(opt.zh, opt.en));
        }
    }

    if (panelMention) {
        panelMention->setTitle(pick(QString::fromUtf8("选择好友"),
                                    QStringLiteral("Select friends")));
    }

    if (panelPrivacy) {
        panelPrivacy->setTitle(pick(QString::fromUtf8("可见范围"),
                                    QStringLiteral("Visibility")));
    }
    for (auto &opt : privacyOptions_) {
        if (opt.button) {
            opt.button->setText(pick(opt.zh, opt.en));
        }
    }

    if (panelShare) {
        panelShare->setTitle(pick(QString::fromUtf8("分享到"),
                                  QStringLiteral("Share to")));
    }

    if (panelVolume) {
        panelVolume->setTitle(pick(QString::fromUtf8("音量"),
                                   QStringLiteral("Volume")));
        if (volumeLabel) {
            volumeLabel->setText(pick(QString::fromUtf8("调整音量"),
                                      QStringLiteral("Adjust volume")));
        }
    }

    updateInputBar();
}

QString PublishPage::tagDisplayText(const QString &key) const
{
    for (const auto &opt : tagOptions_) {
        if (opt.key == key || opt.zh == key || opt.en.compare(key, Qt::CaseInsensitive) == 0) {
            return LanguageManager::instance().pick(opt.zh, opt.en);
        }
    }
    return key;
}

QString PublishPage::tagKeyForLabel(const QString &label) const
{
    for (const auto &opt : tagOptions_) {
        if (opt.key == label || opt.zh == label || opt.en.compare(label, Qt::CaseInsensitive) == 0) {
            return opt.key;
        }
    }
    return label;
}


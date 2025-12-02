#include "chat_page.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSizePolicy>
#include <QScrollArea>
#include <QScrollBar>
#include <QSpacerItem>
#include <QTimer>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QBoxLayout>
#include <algorithm>

namespace {
QString defaultChatStyle() {
    return QStringLiteral(
        "QWidget#chatPageRoot { background-color: #01040a; }"
        "QFrame#chatSurface {"
        "  background-color: #040915;"
        "  border-radius: 36px;"
        "  border: 1px solid rgba(47,141,255,0.2);"
        "}"
        "QFrame#panel {"
        "  background-color: rgba(6,12,22,0.95);"
        "  border-radius: 26px;"
        "  border: 1px solid rgba(47,141,255,0.18);"
        "}"
        "QLabel#sectionTitle { font-size: 13px; color: #6d86b4; letter-spacing: 0.25em; }"
        "QLabel#contactName { font-size: 26px; font-weight: 700; color: white; }"
        "QLabel#status { color: #7c94c3; }"
        "QWidget#messageStream {"
        "  background-color: #050b18;"
        "  border-radius: 26px;"
        "  border: 1px solid rgba(47,141,255,0.12);"
        "}"
        "QLineEdit#messageInput {"
        "  background-color: #0b1324;"
        "  border: 1px solid rgba(47,141,255,0.35);"
        "  border-radius: 24px;"
        "  padding: 15px 22px;"
        "  color: white;"
        "}"
        "QLineEdit#messageInput:focus { border: 1px solid rgba(79,161,255,0.85); }"
        "QPushButton#sendButton {"
        "  background-color: #2f8dff;"
        "  color: white;"
        "  padding: 14px 30px;"
        "  border-radius: 24px;"
        "  border: none;"
        "  font-weight: 700;"
        "}"
        "QPushButton#sendButton:hover { background-color: #4ca2ff; }"
        "QListWidget#chatList {"
        "  background-color: transparent;"
        "  border: none;"
        "  color: white;"
        "  font-size: 15px;"
        "}"
        "QListWidget#chatList::item { margin: 4px 0; padding: 10px 8px; border-radius: 12px; }"
        "QListWidget#chatList::item:selected { background-color: rgba(47,141,255,0.2); }"
        "QLabel#hint { color: #5c729c; font-size: 13px; }"
        "QWidget#messageRow { background: transparent; }"
        "QFrame#bubbleOutgoing {"
        "  background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #2f8dff, stop:1 #1a62ff);"
        "  border-radius: 22px;"
        "  padding: 14px 18px;"
        "}"
        "QFrame#bubbleIncoming {"
        "  background-color: #0c121f;"
        "  border-radius: 22px;"
        "  padding: 14px 18px;"
        "}"
        "QFrame#bubbleOutgoing QLabel, QFrame#bubbleIncoming QLabel { color: white; font-size: 14px; }");
}

QString highContrastChatStyle() {
    return QStringLiteral(
        "QWidget#chatPageRoot { background-color: #000; }"
        "QFrame#chatSurface {"
        "  background-color: #111;"
        "  border-radius: 36px;"
        "  border: 2px solid #f4c430;"
        "}"
        "QFrame#panel {"
        "  background-color: #0d0d0d;"
        "  border-radius: 26px;"
        "  border: 2px solid #f4c430;"
        "}"
        "QLabel#sectionTitle { font-size: 13px; color: #f4c430; letter-spacing: 0.25em; }"
        "QLabel#contactName { font-size: 26px; font-weight: 700; color: #ffffff; }"
        "QLabel#status { color: #f9f871; }"
        "QWidget#messageStream {"
        "  background-color: #050505;"
        "  border-radius: 26px;"
        "  border: 2px solid #f4c430;"
        "}"
        "QLineEdit#messageInput {"
        "  background-color: #000;"
        "  border: 2px solid #f4c430;"
        "  border-radius: 24px;"
        "  padding: 15px 22px;"
        "  color: #ffffff;"
        "}"
        "QPushButton#sendButton {"
        "  background-color: #f4c430;"
        "  color: #000;"
        "  padding: 14px 30px;"
        "  border-radius: 24px;"
        "  border: none;"
        "  font-weight: 700;"
        "}"
        "QListWidget#chatList {"
        "  background-color: transparent;"
        "  border: none;"
        "  color: #ffffff;"
        "  font-size: 15px;"
        "}"
        "QListWidget#chatList::item:selected { background-color: rgba(244,196,48,0.3); }"
        "QLabel#hint { color: #f9f871; font-size: 13px; }"
        "QFrame#bubbleOutgoing {"
        "  background-color: #f4c430;"
        "  border-radius: 22px;"
        "  padding: 14px 18px;"
        "}"
        "QFrame#bubbleIncoming {"
        "  background-color: #0f0f0f;"
        "  border-radius: 22px;"
        "  padding: 14px 18px;"
        "  border: 2px solid #f4c430;"
        "}"
        "QFrame#bubbleOutgoing QLabel { color: #000; font-size: 14px; }"
        "QFrame#bubbleIncoming QLabel { color: #ffffff; font-size: 14px; }");
}

struct ChatCopy {
    QString chatsTitle;
    QString placeholder;
    QString sendLabel;
    QString hint;
};

QFrame *createBubble(const QString &text, bool outgoing, QWidget *parent = nullptr) {
    auto *bubble = new QFrame(parent);
    bubble->setObjectName(outgoing ? "bubbleOutgoing" : "bubbleIncoming");

    auto *layout = new QHBoxLayout(bubble);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *label = new QLabel(text, bubble);
    label->setWordWrap(true);
    label->setMinimumWidth(120);
    label->setMaximumWidth(380);
    layout->addWidget(label);
    return bubble;
}
} // namespace

ChatPage::ChatPage(QWidget *parent) : QWidget(parent) {
    setObjectName("chatPageRoot");
    defaultStyleSheet_ = defaultChatStyle();
    highContrastStyleSheet_ = highContrastChatStyle();
    setStyleSheet(defaultStyleSheet_);

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(32, 32, 32, 32);
    rootLayout->setSpacing(16);

    auto *surface = new QFrame(this);
    surface->setObjectName("chatSurface");
    surfaceLayout_ = new QHBoxLayout(surface);
    surfaceLayout_->setContentsMargins(28, 28, 28, 28);
    surfaceLayout_->setSpacing(20);

    listPanel_ = new QFrame(surface);
    listPanel_->setObjectName("panel");
    listPanel_->setMinimumWidth(240);
    auto *listLayout = new QVBoxLayout(listPanel_);
    listLayout->setContentsMargins(20, 20, 20, 20);
    listLayout->setSpacing(14);

    listTitleLabel_ = new QLabel("CHATS", listPanel_);
    listTitleLabel_->setObjectName("sectionTitle");

    chatList_ = new QListWidget(listPanel_);
    chatList_->setObjectName("chatList");

    listLayout->addWidget(listTitleLabel_);
    listLayout->addWidget(chatList_);

    conversationPanel_ = new QFrame(surface);
    conversationPanel_->setObjectName("panel");
    auto *conversationLayout = new QVBoxLayout(conversationPanel_);
    conversationLayout->setContentsMargins(24, 24, 24, 24);
    conversationLayout->setSpacing(18);

    auto *header = new QHBoxLayout();
    header->setSpacing(12);
    contactLabel_ = new QLabel("—", conversationPanel_);
    contactLabel_->setObjectName("contactName");
    statusLabel_ = new QLabel("", conversationPanel_);
    statusLabel_->setObjectName("status");
    header->addWidget(contactLabel_);
    header->addWidget(statusLabel_);
    header->addStretch();

    messageScroll_ = new QScrollArea(conversationPanel_);
    messageScroll_->setWidgetResizable(true);
    messageScroll_->setFrameShape(QFrame::NoFrame);
    messageScroll_->setStyleSheet("QScrollArea { background: transparent; border: none; }");

    auto *messageContainer = new QWidget(messageScroll_);
    messageContainer->setObjectName("messageStream");
    messageLayout_ = new QVBoxLayout(messageContainer);
    messageLayout_->setSpacing(14);
    messageLayout_->setContentsMargins(18, 18, 18, 18);
    messageSpacer_ = new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);
    messageLayout_->addItem(messageSpacer_);

    messageScroll_->setWidget(messageContainer);

    auto *inputArea = new QWidget(conversationPanel_);
    auto *inputAreaLayout = new QVBoxLayout(inputArea);
    inputAreaLayout->setContentsMargins(0, 0, 0, 0);
    inputAreaLayout->setSpacing(12);

    auto *inputRow = new QHBoxLayout();
    inputRow->setSpacing(12);
    messageInput_ = new QLineEdit(inputArea);
    messageInput_->setObjectName("messageInput");
    messageInput_->setPlaceholderText("Share a RealMessage...");
    sendButton_ = new QPushButton("Send", inputArea);
    sendButton_->setObjectName("sendButton");
    inputRow->addWidget(messageInput_, 1);
    inputRow->addWidget(sendButton_);

    hintLabel_ = new QLabel("Press Enter ↵ to send instantly", inputArea);
    hintLabel_->setObjectName("hint");

    inputAreaLayout->addLayout(inputRow);
    inputAreaLayout->addWidget(hintLabel_);

    conversationLayout->addLayout(header);
    conversationLayout->addWidget(messageScroll_, 1);
    conversationLayout->addWidget(inputArea);

    surfaceLayout_->addWidget(listPanel_, 1);
    surfaceLayout_->addWidget(conversationPanel_, 2);

    rootLayout->addWidget(surface);

    connect(sendButton_, &QPushButton::clicked, this, &ChatPage::handleSend);
    connect(messageInput_, &QLineEdit::returnPressed, this, &ChatPage::handleSend);
    connect(chatList_, &QListWidget::currentRowChanged, this, &ChatPage::handleContactChanged);

    initializeContacts();
    rebuildStrings();
    updateResponsiveLayout();
}

void ChatPage::appendMessageRow(const QString &text, bool outgoing) {
    if (!messageLayout_ || !messageScroll_) {
        return;
    }

    auto *bubble = createBubble(text, outgoing, messageScroll_->widget());
    auto *rowWidget = new QWidget(messageScroll_->widget());
    rowWidget->setObjectName("messageRow");
    auto *rowLayout = new QHBoxLayout(rowWidget);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(8);

    if (outgoing) {
        rowLayout->addStretch();
        rowLayout->addWidget(bubble, 0, Qt::AlignRight);
    } else {
        rowLayout->addWidget(bubble, 0, Qt::AlignLeft);
        rowLayout->addStretch();
    }

    const int insertIndex = std::max(0, messageLayout_->count() - 1);
    messageLayout_->insertWidget(insertIndex, rowWidget);

    QTimer::singleShot(0, this, [this]() {
        if (messageScroll_->verticalScrollBar()) {
            messageScroll_->verticalScrollBar()->setValue(messageScroll_->verticalScrollBar()->maximum());
        }
    });
}

void ChatPage::clearConversation() {
    if (!messageLayout_) {
        return;
    }
    while (messageLayout_->count() > 1) {
        QLayoutItem *item = messageLayout_->takeAt(0);
        if (auto *widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
}

void ChatPage::rebuildConversation() {
    if (contacts_.empty()) {
        return;
    }
    clearConversation();
    const auto &thread = contacts_.at(activeContactIndex_);
    for (const auto &message : thread.messages) {
        appendMessageRow(message.text, message.outgoing);
    }
}

void ChatPage::initializeContacts() {
    contacts_.clear();
    if (language_ == AppLanguage::English) {
    contacts_ = {
        {"Noah Chen", "active · 2m ago", {
             {"omw to the rooftop drop, you coming?", false},
             {"give me 5, need to finish today’s BeReal edit", true}
         }},
        {"Zaya Tran", "drafting a sunset drop", {
             {"Need your RAWs from Lisbon?", false},
             {"Uploading now — check the shared drive.", true}
         }},
        {"Studio crew", "last seen · 5m ago", {
             {"Standup in 10, bring the FPV build", false}
         }},
        {"Fam group chat", "muted", {
             {"Call when you land 💙", false}
         }}
    };
    } else {
        contacts_ = {
            {QString::fromUtf8("陈诺亚"), QString::fromUtf8("在线 · 2 分钟前"), {
                 {QString::fromUtf8("我在去屋顶实拍的路上，你来吗？"), false},
                 {QString::fromUtf8("再给我 5 分钟，剪完今天的 BeReal 就出发"), true}
             }},
            {QString::fromUtf8("Zaya"), QString::fromUtf8("准备日落实拍"), {
                 {QString::fromUtf8("里斯本的 RAW 素材还留着吗？"), false},
                 {QString::fromUtf8("正在上传，去共享盘收一下～"), true}
             }},
            {QString::fromUtf8("工作室小队"), QString::fromUtf8("5 分钟前在线"), {
                 {QString::fromUtf8("10 分钟后站会，记得带 FPV 套件"), false}
             }},
            {QString::fromUtf8("家人群"), QString::fromUtf8("免打扰"), {
                 {QString::fromUtf8("落地记得报平安 💙"), false}
             }}
        };
    }

    chatList_->clear();
    for (const auto &contact : contacts_) {
        chatList_->addItem(contact.name);
    }

    if (!contacts_.empty()) {
        chatList_->setCurrentRow(0);
        switchContact(0);
    }
}

void ChatPage::switchContact(int index) {
    if (index < 0 || index >= static_cast<int>(contacts_.size())) {
        return;
    }
    activeContactIndex_ = index;
    contactLabel_->setText(contacts_[index].name);
    statusLabel_->setText(contacts_[index].status);
    rebuildConversation();
}

void ChatPage::handleSend() {
    if (!messageInput_ || contacts_.empty()) {
        return;
    }
    const QString text = messageInput_->text().trimmed();
    if (text.isEmpty()) {
        return;
    }

    auto &thread = contacts_.at(activeContactIndex_);
    thread.messages.push_back({text, true});
    appendMessageRow(text, true);
    messageInput_->clear();
}

void ChatPage::handleContactChanged(int row) {
    switchContact(row);
}

void ChatPage::updateResponsiveLayout() {
    if (!surfaceLayout_ || !listPanel_ || !conversationPanel_) {
        return;
    }
    surfaceLayout_->setDirection(QBoxLayout::LeftToRight);
    listPanel_->setMaximumWidth(320);
    listPanel_->setMinimumWidth(220);
}

void ChatPage::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    updateResponsiveLayout();
}

void ChatPage::setLanguage(AppLanguage language) {
    if (language_ == language) {
        rebuildStrings();
        initializeContacts();
        return;
    }
    language_ = language;
    rebuildStrings();
    initializeContacts();
}

void ChatPage::rebuildStrings() {
    ChatCopy copy;
    if (language_ == AppLanguage::English) {
        copy = {
            "CHATS",
            "Share a RealMessage...",
            "Send",
            "Press Enter ↵ to send instantly"
        };
    } else {
        copy = {
            QString::fromUtf8("聊天列表"),
            QString::fromUtf8("和好友分享一条实时讯息…"),
            QString::fromUtf8("发送"),
            QString::fromUtf8("按 Enter ↵ 即刻发送")
        };
    }

    chatsTitle_ = copy.chatsTitle;
    messagePlaceholder_ = copy.placeholder;
    sendButtonLabel_ = copy.sendLabel;
    hintText_ = copy.hint;

    if (listTitleLabel_) {
        listTitleLabel_->setText(chatsTitle_);
    }
    if (messageInput_) {
        messageInput_->setPlaceholderText(messagePlaceholder_);
    }
    if (sendButton_) {
        sendButton_->setText(sendButtonLabel_);
    }
    if (hintLabel_) {
        hintLabel_->setText(hintText_);
    }
}

void ChatPage::setHighContrastMode(bool enabled) {
    if (highContrastMode_ == enabled) {
        return;
    }
    highContrastMode_ = enabled;
    setStyleSheet(enabled ? highContrastStyleSheet_ : defaultStyleSheet_);
}


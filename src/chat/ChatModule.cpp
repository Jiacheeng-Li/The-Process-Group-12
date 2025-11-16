#include "ChatModule.h"
#include "Profile.h"
#include <QFont>
#include <QStyleOption>
#include <QPainter>

// 简化聊天列表项（仅显示名称）
ChatListItem::ChatListItem(const QString &name, QWidget *parent)
    : QWidget(parent) {
    setFixedHeight(60);
    setStyleSheet("hover { background-color: #f0f0f0; }");
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 10, 0);
    layout->setSpacing(10);

    // 简化头像
    QWidget *avatar = new QWidget();
    avatar->setFixedSize(40, 40);
    avatar->setStyleSheet("background-color: #d9d9d9; border-radius: 20px;");
    layout->addWidget(avatar);

    // 名称
    QLabel *nameLbl = new QLabel(name);
    nameLbl->setObjectName("contactName");
    nameLbl->setFont(QFont("", 14, QFont::Normal));
    layout->addWidget(nameLbl);
    layout->addStretch();
}

// 简化聊天详情页（微信风格）
ChatDetail::ChatDetail(const QString &name, QWidget *parent)
    : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 顶部导航栏
    QWidget *nav = new QWidget();
    nav->setFixedHeight(44);
    nav->setStyleSheet("background-color: #f7f7f7; border-bottom: 1px solid #e0e0e0;");
    QHBoxLayout *navLayout = new QHBoxLayout(nav);
    navLayout->setContentsMargins(10, 0, 10, 0);

    QLabel *nameLbl = new QLabel(name);
    nameLbl->setFont(QFont("", 16, QFont::Normal));
    navLayout->addWidget(nameLbl);
    navLayout->addStretch();
    mainLayout->addWidget(nav);

    // 消息列表
    m_msgList = new QListWidget();
    m_msgList->setStyleSheet("background-color: #ece5dd; border: none;");
    m_msgList->setSpacing(10);
    mainLayout->addWidget(m_msgList, 1);

    // 输入区
    QWidget *inputArea = new QWidget();
    inputArea->setFixedHeight(50);
    inputArea->setStyleSheet("background-color: #f7f7f7; border-top: 1px solid #e0e0e0;");
    QHBoxLayout *inputLayout = new QHBoxLayout(inputArea);
    inputLayout->setContentsMargins(10, 5, 10, 5);
    inputLayout->setSpacing(10);

    m_inputEdit = new QLineEdit();
    m_inputEdit->setPlaceholderText("Enter Message...");
    m_inputEdit->setStyleSheet(R"(
        QLineEdit {
            background-color: white;
            border: 1px solid #e0e0e0;
            border-radius: 20px;
            padding: 0 15px;
            font-size: 14px;
        }
    )");
    inputLayout->addWidget(m_inputEdit, 1);

    QPushButton *sendBtn = new QPushButton("Send");
    sendBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #07c160;
            color: white;
            border: none;
            border-radius: 20px;
            padding: 0 20px;
            font-size: 14px;
        }
    )");
    sendBtn->setDisabled(true);
    connect(m_inputEdit, &QLineEdit::textChanged, [sendBtn](const QString &text) {
        sendBtn->setEnabled(!text.trimmed().isEmpty());
    });
    connect(sendBtn, &QPushButton::clicked, this, &ChatDetail::sendMessage);
    inputLayout->addWidget(sendBtn);

    mainLayout->addWidget(inputArea);
}

void ChatDetail::sendMessage() {
    QString text = m_inputEdit->text().trimmed();
    if (text.isEmpty()) return;

    // 简化消息添加逻辑
    QListWidgetItem *item = new QListWidgetItem(m_msgList);
    item->setText(text);
    item->setTextAlignment(Qt::AlignRight);
    item->setForeground(Qt::white);
    item->setBackground(QColor("#07c160"));
    item->setSizeHint(QSize(0, 40));

    m_inputEdit->clear();
    m_msgList->scrollToBottom();
}

// 微信风格聊天主页面
ChatMain::ChatMain(QWidget *parent)
    : QWidget(parent), m_currentWidget(nullptr) {
    initUI();
}

void ChatMain::initUI() {
    setStyleSheet("background-color: #f7f7f7;");
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(1);

    // 左侧聊天列表
    m_chatList = new QListWidget();
    m_chatList->setFixedWidth(240);
    m_chatList->setStyleSheet(R"(
        QListWidget { background-color: white; border: none; }
        QListWidget::item { border-bottom: 1px solid #f0f0f0; }
        QListWidget::item:selected { background-color: #f0f0f0; border: none; }
    )");
    mainLayout->addWidget(m_chatList);

    // 右侧容器
    m_rightContainer = new QWidget();
    m_rightContainer->setStyleSheet("background-color: #ece5dd;");
    mainLayout->addWidget(m_rightContainer, 1);

    // 给右侧容器安装事件过滤器（监听大小变化）
    m_rightContainer->installEventFilter(this);

    // 添加测试联系人
    QString names[] = {"Alex", "Victor", "Michael", "Zelretch", "QtGroup"};
    for (QString name : names) {
        QListWidgetItem *item = new QListWidgetItem(m_chatList);
        item->setSizeHint(QSize(0, 60));
        m_chatList->setItemWidget(item, new ChatListItem(name));
    }

    // 点击联系人显示聊天详情
    connect(m_chatList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        ChatListItem *widget = qobject_cast<ChatListItem*>(m_chatList->itemWidget(item));
        if (!widget) return;

        QLabel *nameLbl = widget->findChild<QLabel*>("contactName");
        if (nameLbl && !nameLbl->text().isEmpty()) {
            showChatDetail(nameLbl->text());
        }
    });

    // 默认显示提示页面
    showDefaultTip();
}

// 事件过滤器：监听右侧容器大小变化
bool ChatMain::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_rightContainer && event->type() == QEvent::Resize) {
        resizeRightWidget(); // 调整右侧控件大小
        return true;
    }
    return QWidget::eventFilter(watched, event);
}

// 统一调整右侧控件大小
void ChatMain::resizeRightWidget() {
    if (m_currentWidget) {
        m_currentWidget->setGeometry(m_rightContainer->rect());
    }
}

// 显示聊天详情
void ChatMain::showChatDetail(const QString &name) {
    // 先删除当前显示的控件
    if (m_currentWidget) {
        m_currentWidget->deleteLater();
        m_currentWidget = nullptr;
    }

    // 创建新的聊天详情页
    m_currentChat = new ChatDetail(name, m_rightContainer);
    m_currentWidget = m_currentChat;

    // 初始大小设置
    resizeRightWidget();
    m_currentWidget->show();
}

// 显示默认提示
void ChatMain::showDefaultTip() {
    // 先删除当前显示的控件
    if (m_currentWidget) {
        m_currentWidget->deleteLater();
        m_currentWidget = nullptr;
    }

    // 创建提示控件
    m_currentWidget = new QWidget(m_rightContainer);
    m_currentWidget->setStyleSheet("background-color: #ece5dd;");

    // 提示内容布局
    QVBoxLayout *tipLayout = new QVBoxLayout(m_currentWidget);
    tipLayout->setAlignment(Qt::AlignCenter);
    tipLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *icon = new QWidget();
    icon->setFixedSize(80, 80);
    icon->setStyleSheet("background-color: #c9c9c9; border-radius: 40px;");
    tipLayout->addWidget(icon);

    QLabel *tipLabel = new QLabel("选择一个聊天开始对话");
    tipLabel->setStyleSheet("font-size: 18px; color: #666; margin-top: 20px;");
    tipLayout->addWidget(tipLabel);

    // 初始大小设置
    resizeRightWidget();
    m_currentWidget->show();
}

// 页面切换管理器
PageSwitcher::PageSwitcher(QWidget *parent)
    : QWidget(parent) {
    m_profilePage = new TikTokProfile();
    m_chatMainPage = new ChatMain();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_currentPage = m_profilePage;
    mainLayout->addWidget(m_currentPage, 1);

    QWidget *tabBar = new QWidget();
    tabBar->setFixedHeight(50);
    tabBar->setStyleSheet("background-color: white; border-top: 1px solid #e0e0e0;");
    QHBoxLayout *tabLayout = new QHBoxLayout(tabBar);
    tabLayout->setContentsMargins(0, 0, 0, 0);

    m_profileBtn = new QPushButton("Profile");
    m_profileBtn->setStyleSheet(R"(
        QPushButton { background: transparent; border: none; font-size: 14px; font-weight: bold; color: #000; }
        QPushButton:hover { color: #07c160; }
    )");
    m_profileBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(m_profileBtn, &QPushButton::clicked, this, &PageSwitcher::switchToProfile);
    tabLayout->addWidget(m_profileBtn);

    m_chatBtn = new QPushButton("Chat");
    m_chatBtn->setStyleSheet(R"(
        QPushButton { background: transparent; border: none; font-size: 14px; color: #666; }
        QPushButton:hover { color: #07c160; }
    )");
    m_chatBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(m_chatBtn, &QPushButton::clicked, this, &PageSwitcher::switchToChat);
    tabLayout->addWidget(m_chatBtn);

    mainLayout->addWidget(tabBar);
}

void PageSwitcher::switchToProfile() {
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(this->layout());
    if (!mainLayout) return;

    if (m_currentPage) {
        mainLayout->removeWidget(m_currentPage);
        m_currentPage->hide();
    }

    m_currentPage = m_profilePage;
    mainLayout->addWidget(m_currentPage, 1);
    m_currentPage->show();

    m_profileBtn->setStyleSheet(R"(
        QPushButton { background: transparent; border: none; font-size: 14px; font-weight: bold; color: #000; }
    )");
    m_chatBtn->setStyleSheet(R"(
        QPushButton { background: transparent; border: none; font-size: 14px; color: #666; }
    )");
}

void PageSwitcher::switchToChat() {
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(this->layout());
    if (!mainLayout) return;

    if (m_currentPage) {
        mainLayout->removeWidget(m_currentPage);
        m_currentPage->hide();
    }

    m_currentPage = m_chatMainPage;
    mainLayout->addWidget(m_currentPage, 1);
    m_currentPage->show();

    m_chatBtn->setStyleSheet(R"(
        QPushButton { background: transparent; border: none; font-size: 14px; font-weight: bold; color: #000; }
    )");
    m_profileBtn->setStyleSheet(R"(
        QPushButton { background: transparent; border: none; font-size: 14px; color: #666; }
    )");
}

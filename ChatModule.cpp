#include "ChatModule.h"
#include "Profile.h" // 包含头文件
#include <QFont>

// 简化聊天列表项（仅显示名称）
ChatListItem::ChatListItem(const QString &name, QWidget *parent)
    : QWidget(parent) {
    setFixedHeight(60);
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 10, 0);
    layout->setSpacing(10);

    // 简化头像
    QWidget *avatar = new QWidget();
    avatar->setFixedSize(40, 40);
    avatar->setStyleSheet("background-color: #e0e0e0; border-radius: 20px;");
    layout->addWidget(avatar);

    // 名称（设置对象名，方便查找）
    QLabel *nameLbl = new QLabel(name);
    nameLbl->setObjectName("contactName");
    nameLbl->setFont(QFont("", 14, QFont::Bold));
    layout->addWidget(nameLbl);
    layout->addStretch();
}

// 简化聊天详情页
ChatDetail::ChatDetail(const QString &name, QWidget *parent)
    : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 顶部导航
    QWidget *nav = new QWidget();
    nav->setFixedHeight(44);
    nav->setStyleSheet("background-color: #f8f8f8; border-bottom: 1px solid #e0e0e0;");
    QHBoxLayout *navLayout = new QHBoxLayout(nav);
    navLayout->setContentsMargins(10, 0, 10, 0);

    QPushButton *backBtn = new QPushButton("← " + name);
    backBtn->setStyleSheet("background: transparent; border: none; font-size: 14px;");
    connect(backBtn, &QPushButton::clicked, this, &ChatDetail::backToChatList);
    navLayout->addWidget(backBtn);
    mainLayout->addWidget(nav);

    // 消息列表
    m_msgList = new QListWidget();
    m_msgList->setSpacing(10);
    mainLayout->addWidget(m_msgList, 1);

    // 输入区
    QWidget *inputArea = new QWidget();
    inputArea->setFixedHeight(50);
    inputArea->setStyleSheet("background-color: #f8f8f8; border-top: 1px solid #e0e0e0;");
    QHBoxLayout *inputLayout = new QHBoxLayout(inputArea);
    inputLayout->setContentsMargins(10, 5, 10, 5);
    inputLayout->setSpacing(10);

    m_inputEdit = new QLineEdit();
    m_inputEdit->setPlaceholderText("输入消息...");
    m_inputEdit->setStyleSheet("border: 1px solid #e0e0e0; border-radius: 20px; padding: 0 15px;");
    inputLayout->addWidget(m_inputEdit, 1);

    QPushButton *sendBtn = new QPushButton("发送");
    sendBtn->setStyleSheet("background-color: #007aff; color: white; border: none; border-radius: 20px; padding: 0 20px;");
    connect(sendBtn, &QPushButton::clicked, this, &ChatDetail::sendMessage);
    inputLayout->addWidget(sendBtn);

    mainLayout->addWidget(inputArea);
}

void ChatDetail::sendMessage() {
    QString text = m_inputEdit->text().trimmed();
    if (text.isEmpty()) return;

    // 添加消息到列表
    QListWidgetItem *item = new QListWidgetItem(m_msgList);
    item->setText(text);
    m_inputEdit->clear();
}

// 简化聊天主页面
ChatMain::ChatMain(QWidget *parent)
    : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 聊天列表
    m_chatList = new QListWidget();
    mainLayout->addWidget(m_chatList);

    // 添加测试联系人
    QString names[] = {"张三", "李四", "王五", "赵六", "Qt交流群"};
    for (QString name : names) {
        QListWidgetItem *item = new QListWidgetItem(m_chatList);
        item->setSizeHint(QSize(0, 60));
        m_chatList->setItemWidget(item, new ChatListItem(name));
    }

    // 点击进入聊天（通过对象名查找，更可靠）
    connect(m_chatList, &QListWidget::itemClicked, [this](QListWidgetItem *item) {
        ChatListItem *widget = qobject_cast<ChatListItem*>(m_chatList->itemWidget(item));
        if (widget) {
            QLabel *nameLbl = widget->findChild<QLabel*>("contactName");
            if (nameLbl) {
                emit openChatDetail(nameLbl->text());
            }
        }
    });
}

// 页面切换管理器
PageSwitcher::PageSwitcher(QWidget *parent)
    : QWidget(parent) {
    // 创建核心页面
    m_profilePage = new TikTokProfile();
    m_chatMainPage = new ChatMain();
    m_chatDetailPage = nullptr;

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 默认显示个人主页
    m_currentPage = m_profilePage;
    mainLayout->addWidget(m_currentPage, 1);

    // 底部切换按钮（存储指针，避免文本查找）
    QWidget *tabBar = new QWidget();
    tabBar->setFixedHeight(50);
    tabBar->setStyleSheet("background-color: #f8f8f8; border-top: 1px solid #e0e0e0;");
    QHBoxLayout *tabLayout = new QHBoxLayout(tabBar);
    tabLayout->setContentsMargins(0, 0, 0, 0);

    // 个人主页按钮
    m_profileBtn = new QPushButton("个人主页");
    m_profileBtn->setStyleSheet("background: transparent; border: none; font-size: 14px; font-weight: bold;");
    m_profileBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(m_profileBtn, &QPushButton::clicked, this, &PageSwitcher::switchToProfile);
    tabLayout->addWidget(m_profileBtn);

    // 聊天按钮
    m_chatBtn = new QPushButton("聊天");
    m_chatBtn->setStyleSheet("background: transparent; border: none; font-size: 14px; color: #666;");
    m_chatBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(m_chatBtn, &QPushButton::clicked, this, &PageSwitcher::switchToChat);
    tabLayout->addWidget(m_chatBtn);

    mainLayout->addWidget(tabBar);

    // 绑定聊天列表点击事件
    connect(m_chatMainPage, &ChatMain::openChatDetail, this, &PageSwitcher::openChatDetailPage);
}

void PageSwitcher::switchToProfile() {
    // 清理聊天详情页
    if (m_currentPage == m_chatDetailPage) {
        delete m_chatDetailPage;
        m_chatDetailPage = nullptr;
    } else {
        m_currentPage->hide();
    }

    // 显示个人主页
    m_currentPage = m_profilePage;
    m_currentPage->show();

    // 更新按钮样式（使用存储的指针）
    m_profileBtn->setStyleSheet("background: transparent; border: none; font-size: 14px; font-weight: bold;");
    m_chatBtn->setStyleSheet("background: transparent; border: none; font-size: 14px; color: #666;");
}

void PageSwitcher::switchToChat() {
    // 清理聊天详情页
    if (m_currentPage == m_chatDetailPage) {
        delete m_chatDetailPage;
        m_chatDetailPage = nullptr;
    } else {
        m_currentPage->hide();
    }

    // 显示聊天主页面
    m_currentPage = m_chatMainPage;
    m_currentPage->show();

    // 更新按钮样式（使用存储的指针）
    m_chatBtn->setStyleSheet("background: transparent; border: none; font-size: 14px; font-weight: bold;");
    m_profileBtn->setStyleSheet("background: transparent; border: none; font-size: 14px; color: #666;");
}

void PageSwitcher::openChatDetailPage(const QString &name) {
    // 隐藏聊天主页面
    m_chatMainPage->hide();

    // 创建聊天详情页
    m_chatDetailPage = new ChatDetail(name);
    m_currentPage = m_chatDetailPage;

    // 修复：使用正确的addWidget参数（仅传递widget，或明确布局类型后指定拉伸因子）
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (mainLayout) {
        mainLayout->insertWidget(0, m_currentPage, 1); // 插入到第0位，占满剩余空间
        m_currentPage->show();
    }

    // 绑定返回信号
    connect(m_chatDetailPage, &ChatDetail::backToChatList, this, &PageSwitcher::backFromChatDetail);
}

void PageSwitcher::backFromChatDetail() {
    // 清理聊天详情页
    delete m_chatDetailPage;
    m_chatDetailPage = nullptr;

    // 显示聊天主页面
    m_currentPage = m_chatMainPage;
    m_currentPage->show();
}

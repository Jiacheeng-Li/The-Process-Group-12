#ifndef CHATMODULE_H
#define CHATMODULE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QLineEdit>

// 前向声明（解决循环引用问题）
class TikTokProfile;

// 简化聊天列表项
class ChatListItem : public QWidget {
    Q_OBJECT
public:
    explicit ChatListItem(const QString &name, QWidget *parent = nullptr);
};

// 简化聊天详情页
class ChatDetail : public QWidget {
    Q_OBJECT
public:
    explicit ChatDetail(const QString &name, QWidget *parent = nullptr);
signals:
    void backToChatList();
private slots:
    void sendMessage();
private:
    QListWidget *m_msgList;
    QLineEdit *m_inputEdit;
};

// 简化聊天主页面
class ChatMain : public QWidget {
    Q_OBJECT
public:
    explicit ChatMain(QWidget *parent = nullptr);
signals:
    void openChatDetail(const QString &name);
private:
    QListWidget *m_chatList;
};

// 页面切换管理器（核心）
class PageSwitcher : public QWidget {
    Q_OBJECT
public:
    explicit PageSwitcher(QWidget *parent = nullptr);
private slots:
    void switchToProfile();
    void switchToChat();
    void openChatDetailPage(const QString &name);
    void backFromChatDetail();
private:
    QWidget *m_currentPage;
    TikTokProfile *m_profilePage; // 前向声明后可正常使用
    ChatMain *m_chatMainPage;
    ChatDetail *m_chatDetailPage;
    // 存储切换按钮指针（避免通过文本查找）
    QPushButton *m_profileBtn;
    QPushButton *m_chatBtn;
};

#endif // CHATMODULE_H

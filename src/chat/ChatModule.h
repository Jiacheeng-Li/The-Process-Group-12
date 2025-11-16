#ifndef CHATMODULE_H
#define CHATMODULE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QResizeEvent>

// 前向声明
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
private slots:
    void sendMessage();
private:
    QListWidget *m_msgList;
    QLineEdit *m_inputEdit;
};

// 微信风格聊天主页面（左侧列表+右侧聊天）
class ChatMain : public QWidget {
    Q_OBJECT
public:
    explicit ChatMain(QWidget *parent = nullptr);
protected:
    // 重写事件过滤器，监听右侧容器大小变化
    bool eventFilter(QObject *watched, QEvent *event) override;
private:
    void initUI();
    void showChatDetail(const QString &name);
    void showDefaultTip();
    void resizeRightWidget(); // 统一调整右侧控件大小
private:
    QListWidget *m_chatList;
    QWidget *m_rightContainer;
    ChatDetail *m_currentChat;
    QWidget *m_currentWidget; // 跟踪右侧当前显示的控件
};

// 页面切换管理器（核心）
class PageSwitcher : public QWidget {
    Q_OBJECT
public:
    explicit PageSwitcher(QWidget *parent = nullptr);
private slots:
    void switchToProfile();
    void switchToChat();
private:
    QWidget *m_currentPage;
    TikTokProfile *m_profilePage;
    ChatMain *m_chatMainPage;
    QPushButton *m_profileBtn;
    QPushButton *m_chatBtn;
};

#endif // CHATMODULE_H

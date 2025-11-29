#ifndef CHAT_PAGE_H
#define CHAT_PAGE_H

#include <QWidget>
#include <QString>
#include <vector>

#include "app_settings.h"

class QVBoxLayout;
class QScrollArea;
class QLineEdit;
class QSpacerItem;
class QListWidget;
class QLabel;
class QHBoxLayout;
class QFrame;
class QResizeEvent;
class QPushButton;

class ChatPage : public QWidget {
    Q_OBJECT

public:
    explicit ChatPage(QWidget *parent = nullptr);
    void setLanguage(AppLanguage language);
    void setHighContrastMode(bool enabled);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void handleSend();
    void handleContactChanged(int row);

private:
    struct Message {
        QString text;
        bool outgoing;
    };

    struct ContactThread {
        QString name;
        QString status;
        std::vector<Message> messages;
    };

    void appendMessageRow(const QString &text, bool outgoing);
    void clearConversation();
    void rebuildConversation();
    void initializeContacts();
    void switchContact(int index);
    void updateResponsiveLayout();
    void rebuildStrings();

    QVBoxLayout *messageLayout_ = nullptr;
    QScrollArea *messageScroll_ = nullptr;
    QLineEdit *messageInput_ = nullptr;
    QSpacerItem *messageSpacer_ = nullptr;
    QLabel *contactLabel_ = nullptr;
    QLabel *statusLabel_ = nullptr;
    QListWidget *chatList_ = nullptr;
    QFrame *listPanel_ = nullptr;
    QFrame *conversationPanel_ = nullptr;
    QHBoxLayout *surfaceLayout_ = nullptr;
    QLabel *listTitleLabel_ = nullptr;
    QPushButton *sendButton_ = nullptr;
    QLabel *hintLabel_ = nullptr;

    std::vector<ContactThread> contacts_;
    int activeContactIndex_ = 0;
    AppLanguage language_ = AppLanguage::Chinese;
    bool highContrastMode_ = false;
    QString defaultStyleSheet_;
    QString highContrastStyleSheet_;
    QString chatsTitle_;
    QString messagePlaceholder_;
    QString sendButtonLabel_;
    QString hintText_;
};

#endif // CHAT_PAGE_H


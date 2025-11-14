#ifndef CHATPAGE_H
#define CHATPAGE_H

#include <QWidget>
#include <QListView>
#include <QLineEdit>
#include <QTcpSocket>

class ChatPage : public QWidget
{
    Q_OBJECT
public:
    explicit ChatPage(QWidget *parent = nullptr);

private:
    QListView *chatListView;
    QLineEdit *messageInput;
    QTcpSocket *socket;
    
    // TODO: Add chat list display
    // TODO: Add message sending
    // TODO: Add real-time updates
};

#endif // CHATPAGE_H

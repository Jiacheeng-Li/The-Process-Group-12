#ifndef PUBLISHPAGE_H
#define PUBLISHPAGE_H

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>

class PublishPage : public QWidget {
    Q_OBJECT
public:
    explicit PublishPage(QWidget *parent = nullptr);

signals:
    void sendPressed();     // 点击 Send 后触发
    void backToRecord();    // 点击 ❌ 后返回录制界面

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    // 第一行
    QLineEdit *inputBar;
    QPushButton *bar5;
    QPushButton *bar6;   // ❌ 重新录制
    QPushButton *bar7;

    // 第二行 - 手机框
    QWidget *phoneFrame;
    QWidget *backCamera;
    QWidget *frontCamera;

    // 第三行
    QPushButton *btn1;
    QPushButton *btn2;
    QPushButton *btn3;
    QPushButton *btn4;

    // 第四行
    QPushButton *sendButton;
};

#endif // PUBLISHPAGE_H

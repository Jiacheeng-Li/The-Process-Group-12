#ifndef PUBLISHPAGE_H
#define PUBLISHPAGE_H

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QSlider>
#include <QStringList>
#include "popuppanel.h"


class PublishPage : public QWidget
{
    Q_OBJECT
public:
    explicit PublishPage(QWidget *parent = nullptr);

signals:
    void backToRecord();
    void sendPressed(const QString &thumbPath);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onTagClicked();
    void onMentionClicked();
    void onPrivacyClicked();
    void onLocationClicked();
    void onShareClicked();
    void onVolumeClicked();
    void onUserTyped();       // <- new

private:
    void updateInputBar();    // <- new

private:
    QLineEdit *inputBar;
    QWidget *phoneFrame;
    QWidget *backCamera;
    QWidget *frontCamera;

    QPushButton *bar5;
    QPushButton *bar6;
    QPushButton *bar7;

    QPushButton *btn1;
    QPushButton *btn2;
    QPushButton *btn3;
    QPushButton *btn4;

    QPushButton *sendButton;

    bool isLocationOn = true;

    PopupPanel *panelTag = nullptr;
    PopupPanel *panelMention = nullptr;
    PopupPanel *panelPrivacy = nullptr;
    PopupPanel *panelShare = nullptr;
    PopupPanel *panelVolume = nullptr;

    // new — data model
    QStringList selectedTags;
    QStringList selectedFriends;
    QString privacySelection = "所有人可见";
    QString userText = "";
    bool blockTextSignal = false;
    QSlider *volumeSlider = nullptr;
};

#endif // PUBLISHPAGE_H

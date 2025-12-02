#ifndef PUBLISHPAGE_H
#define PUBLISHPAGE_H

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QSlider>
#include <QStringList>
#include <QVector>
#include "popuppanel.h"

class QAbstractButton;
class QLabel;

class PublishPage : public QWidget
{
    Q_OBJECT
public:
    explicit PublishPage(QWidget *parent = nullptr);
    void loadDraft(const QString &draftText);  // Load draft content

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
    void applyLanguage();
    QString tagDisplayText(const QString &key) const;
    QString tagKeyForLabel(const QString &label) const;

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
    QString privacySelection = "all";
    QString userText = "";
    QString draftBuffer;       // New addition
    bool blockTextSignal = false;
    QSlider *volumeSlider = nullptr;
    QLabel *volumeLabel = nullptr;

    struct LocalizedOption {
        QString key;
        QString zh;
        QString en;
        QAbstractButton *button = nullptr;
    };
    QVector<LocalizedOption> tagOptions_;
    QVector<LocalizedOption> privacyOptions_;
};

#endif // PUBLISHPAGE_H

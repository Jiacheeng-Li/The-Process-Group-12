#ifndef RECORDPAGE_H
#define RECORDPAGE_H

#include <QWidget>
#include <QPushButton>
#include <QTimer>
#include <QEvent>

#include "../shared/language_manager.h"

class RecordPage : public QWidget
{
    Q_OBJECT
public:
    explicit RecordPage(QWidget *parent = nullptr);

signals:
    // After recording ends → go to publish page
    void recordingFinished();   
    // After selecting draft → go to publish page
    void draftSelected(const QString &draftText);  

public:
    static void addDraft(const QString &draftText);  // Helper used by PublishPage to push drafts

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onRecordButtonClicked();
    void onSwitchButtonClicked();
    void onPauseButtonClicked();   
    void onDraftButtonClicked();  // Open draft box

private:
    QWidget *phoneFrame;
    QWidget *backCamera;
    QWidget *frontCamera;

    QPushButton *recordButton;
    QPushButton *switchButton;
    QPushButton *pauseButton;
    QPushButton *draftButton;  // Draft box button
    bool isPaused = false;


    QTimer pulseTimer;
    bool isRecording = false;
    bool backIsBlack = true;
    int pulseState = 0;   // Breathing light gradient

    void applyTranslations(AppLanguage lang);
};

#endif // RECORDPAGE_H

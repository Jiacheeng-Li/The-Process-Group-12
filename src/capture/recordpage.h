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
    void recordingFinished();   // 录制结束后 → 进入发布页
    void draftSelected(const QString &draftText);  // 选择草稿后 → 进入发布页

public:
    static void addDraft(const QString &draftText);  // Helper used by PublishPage to push drafts

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onRecordButtonClicked();
    void onSwitchButtonClicked();
    void onPauseButtonClicked();   // ★ 新增
    void onDraftButtonClicked();  // 打开草稿箱

private:
    QWidget *phoneFrame;
    QWidget *backCamera;
    QWidget *frontCamera;

    QPushButton *recordButton;
    QPushButton *switchButton;
    QPushButton *pauseButton;
    QPushButton *draftButton;  // 草稿箱按钮
    bool isPaused = false;


    QTimer pulseTimer;
    bool isRecording = false;
    bool backIsBlack = true;
    int pulseState = 0;   // 呼吸灯渐变

    void applyTranslations(AppLanguage lang);
};

#endif // RECORDPAGE_H

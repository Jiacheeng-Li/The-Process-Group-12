#ifndef RECORDPAGE_H
#define RECORDPAGE_H

#include <QWidget>
#include <QPushButton>
#include <QTimer>

class RecordPage : public QWidget
{
    Q_OBJECT
public:
    explicit RecordPage(QWidget *parent = nullptr);

signals:
    void recordingFinished();   // 录制结束后 → 进入发布页

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onRecordButtonClicked();
    void onSwitchButtonClicked();

private:
    QWidget *phoneFrame;
    QWidget *backCamera;
    QWidget *frontCamera;

    QPushButton *recordButton;
    QPushButton *switchButton;

    QTimer pulseTimer;
    bool isRecording = false;
    bool backIsBlack = true;
    int pulseState = 0;   // 呼吸灯渐变
};

#endif // RECORDPAGE_H

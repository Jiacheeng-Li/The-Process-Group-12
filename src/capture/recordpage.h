#ifndef RECORDPAGE_H
#define RECORDPAGE_H

#include <QWidget>
#include <QPushButton>
#include <QTimer>

class RecordPage : public QWidget {
    Q_OBJECT

signals:
    void recordingFinished();   // 之后可以用于跳转发布页

public:
    explicit RecordPage(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onRecordButtonClicked();
    void onSwitchButtonClicked();   // 切换前 / 后“摄像头”

private:
    QWidget *phoneFrame;    // 手机外框
    QWidget *backCamera;    // 大矩形：后置摄像头区域
    QWidget *frontCamera;   // 小矩形：前置摄像头区域

    QPushButton *recordButton;  // 红色录制按钮（在黑色区域底部中间）
    QPushButton *switchButton;  // 右下角切换按钮

    bool isRecording = false;
    bool backIsBlack = true;    // true: 大矩形黑，小矩形灰；false: 反之

    QTimer pulseTimer;
    int pulseState = 0;
};

#endif // RECORDPAGE_H

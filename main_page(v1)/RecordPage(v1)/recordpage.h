// zyx recordpage.h
#ifndef RECORDPAGE_H
#define RECORDPAGE_H

#include <QWidget>
#include <QCamera>
#include <QCameraViewfinder>
#include <QMediaRecorder>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>

class RecordPage : public QWidget {
    Q_OBJECT
signals:
    void recordingFinished(const QString &videoPath);  // zyx新增：录制完成信号

public:
    explicit RecordPage(QWidget *parent = nullptr);

private slots:
    void startRecording();
    void stopRecording();
    void publishVideo();

protected:
    void resizeEvent(QResizeEvent *event) override;     // zyx 新增：响应窗口自适应

private:
    QCamera *camera;
    QCameraViewfinder *viewfinder;
    QMediaRecorder *recorder;
    QPushButton *startButton;
    QPushButton *stopButton;
    QPushButton *publishButton;
};

#endif // RECORDPAGE_H

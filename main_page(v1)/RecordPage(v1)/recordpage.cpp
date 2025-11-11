// zyx recordpage.cpp
#include "recordpage.h"
#include <QUrl>      // new
#include <QResizeEvent>    // new

RecordPage::RecordPage(QWidget *parent) : QWidget(parent) {
    camera = new QCamera(this);
    viewfinder = new QCameraViewfinder(this);

    //  zyx新增：自适应显示
    viewfinder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    viewfinder->setAspectRatioMode(Qt::KeepAspectRatio);

    recorder = new QMediaRecorder(camera, this);

    startButton = new QPushButton("Start Recording");
    stopButton = new QPushButton("Stop Recording");
    publishButton = new QPushButton("Publish");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(viewfinder);
    layout->addWidget(startButton);
    layout->addWidget(stopButton);
    layout->addWidget(publishButton);
    setLayout(layout);

    camera->setViewfinder(viewfinder);

    // zyx 修改：连接信号槽
    connect(startButton, &QPushButton::clicked, this, &RecordPage::startRecording);
    connect(stopButton, &QPushButton::clicked, this, &RecordPage::stopRecording);
    connect(publishButton, &QPushButton::clicked, this, &RecordPage::publishVideo);
}

void RecordPage::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    viewfinder->resize(this->size());  // zyx 新增：窗口自适应
}

void RecordPage::startRecording() {
    camera->start();
    recorder->record();
    QMessageBox::information(this, "Recording", "Recording started!");
}

void RecordPage::stopRecording() {
    recorder->stop();
    camera->stop();

    // zyx 修改：触发信号给主窗口（不直接弹窗）
    emit recordingFinished(recorder->outputLocation().toLocalFile());
}

void RecordPage::publishVideo() {
    QMessageBox::information(this, "Publish", "Video uploaded successfully (mock).");
}

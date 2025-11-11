// zyx publishpage.h
#ifndef PUBLISHPAGE_H
#define PUBLISHPAGE_H

#include <QWidget>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>

class PublishPage : public QWidget {
    Q_OBJECT
public:
    explicit PublishPage(const QString &videoPath, QWidget *parent = nullptr);

signals:
    void backRequested();  // zyx new ：返回上级信号

private slots:
    void publishVideo();

private:
    QMediaPlayer *player;
};

#endif // PUBLISHPAGE_H

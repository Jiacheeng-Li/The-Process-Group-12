#ifndef VIDEOVIEWER_H
#define VIDEOVIEWER_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>

class VideoViewer : public QWidget
{
    Q_OBJECT

public:
    explicit VideoViewer(const QString &coverPath, QWidget *parent = nullptr);

signals:
    void closed();      // Close window
    void shared();      // Click share
    void liked();       // Click like

private slots:
    void onLikePressed();
    void onSharePressed();
    void onClosePressed();

private:
    QLabel *videoLabel;
    QPushButton *closeBtn;
    QPushButton *likeBtn;
    QPushButton *shareBtn;

    int likeCount = 0;
    int shareCount = 0;
};

#endif // VIDEOVIEWER_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPixmap>

// 提前声明空窗口类
class VideoWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    // 缩略图点击事件的处理函数
    void onThumbnailClicked();

private:
    QLabel *m_backgroundLabel;
    QLabel *m_videoThumbnail;
    QPixmap m_earthPix;
    QPixmap m_thumbnailPix;
    VideoWindow *m_videoWindow; // 空窗口指针
};

#endif // MAINWINDOW_H

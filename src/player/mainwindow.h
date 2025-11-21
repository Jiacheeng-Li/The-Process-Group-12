#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPixmap>
#include <vector>
#include "the_button.h"

// 提前声明
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
    void onThumbnailClicked();

private:
    // 视频加载函数
    std::vector<TheButtonInfo> getInfoIn(std::string loc);
    
    QLabel *m_backgroundLabel;
    QLabel *m_videoThumbnail;
    QPixmap m_earthPix;
    QPixmap m_thumbnailPix;
    VideoWindow *m_videoWindow;
    
    // 视频列表
    std::vector<TheButtonInfo> m_videos;
};

#endif // MAINWINDOW_H
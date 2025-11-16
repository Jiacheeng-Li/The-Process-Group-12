#ifndef VIDEOWINDOW_H
#define VIDEOWINDOW_H

#include <QWidget>

class VideoWindow : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWindow(QWidget *parent = nullptr);
};

#endif // VIDEOWINDOW_H
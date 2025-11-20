#include "mainwindow.h"
#include "videowindow.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QMouseEvent>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QImageReader>
#include <QMessageBox>
#include <QApplication>

std::vector<TheButtonInfo> MainWindow::getInfoIn(std::string loc) {
    std::vector<TheButtonInfo> out = std::vector<TheButtonInfo>();
    QDir dir(QString::fromStdString(loc));
    
    if (!dir.exists()) {
        qDebug() << "Directory does not exist:" << QString::fromStdString(loc);
        return out;
    }
    
    QDirIterator it(dir);

    while (it.hasNext()) {
        QString f = it.next();
        if (f.contains(".")) {
#if defined(_WIN32)
            if (f.contains(".wmv")) {
#else
            if (f.contains(".mp4") || f.contains(".MOV")) {
#endif
                QString thumb = f.left(f.length() - 4) + ".png";
                if (QFile(thumb).exists()) {
                    QImageReader *imageReader = new QImageReader(thumb);
                    QImage sprite = imageReader->read();
                    if (!sprite.isNull()) {
                        QIcon* ico = new QIcon(QPixmap::fromImage(sprite));
                        QUrl* url = new QUrl(QUrl::fromLocalFile(f));
                        out.push_back(TheButtonInfo(url, ico));
                        qDebug() << "Loaded video:" << f;
                    } else {
                        qDebug() << "warning: skipping video because I couldn't process thumbnail " << thumb;
                    }
                } else {
                    qDebug() << "warning: skipping video because I couldn't find thumbnail " << thumb;
                    // 即使没有缩略图，也加载视频
                    QIcon* ico = new QIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
                    QUrl* url = new QUrl(QUrl::fromLocalFile(f));
                    out.push_back(TheButtonInfo(url, ico));
                    qDebug() << "Loaded video without thumbnail:" << f;
                }
            }
        }
    }
    
    qDebug() << "Total videos loaded:" << out.size();
    return out;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_videoWindow(nullptr)
{
    // 1. 基础设置
    setWindowTitle("main page");
    setMinimumSize(320, 480);
    setMaximumSize(1080, 1920);

    // 2. 尝试加载视频
    m_videos = getInfoIn("F:/Qt/labwork/2811_cw3-master-release-lowres/videos");
    
    // 测试：创建一个示例视频（如果没有视频文件）
    if (m_videos.empty()) {
        qDebug() << "No videos found in specified directory, using default icon";
        // 创建一个默认的视频信息
        QIcon* defaultIcon = new QIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
        QUrl* defaultUrl = new QUrl(QUrl::fromLocalFile("default_video.mp4"));
        m_videos.push_back(TheButtonInfo(defaultUrl, defaultIcon));
    }

    // 3. 加载图片资源
    m_earthPix = QPixmap(":/resources/earth.png");
    if (m_earthPix.isNull()) {
        qDebug() << "Failed to load earth.png, creating default background";
        // 创建默认背景
        m_earthPix = QPixmap(400, 400);
        m_earthPix.fill(Qt::darkBlue);
    }

    m_thumbnailPix = QPixmap(":/resources/video_thumbnail.png");
    if (m_thumbnailPix.isNull()) {
        qDebug() << "Failed to load video_thumbnail.png, creating default thumbnail";
        // 创建默认缩略图
        m_thumbnailPix = QPixmap(200, 150);
        m_thumbnailPix.fill(Qt::red);
    }

    // 4. 背景标签
    m_backgroundLabel = new QLabel(this);
    m_backgroundLabel->setAlignment(Qt::AlignCenter);
    m_backgroundLabel->setScaledContents(false);

    // 5. 视频缩略图
    m_videoThumbnail = new QLabel(this);
    m_videoThumbnail->setAlignment(Qt::AlignCenter);
    m_videoThumbnail->setStyleSheet(
        "border: 2px solid white; "
        "border-radius: 8px; "
        "cursor: pointer;"
        );
    m_videoThumbnail->setScaledContents(false);
    m_videoThumbnail->installEventFilter(this);

    // 6. 布局管理
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addStretch(25);
    mainLayout->addWidget(m_videoThumbnail, 0, Qt::AlignHCenter);
    mainLayout->addStretch(45);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    setCentralWidget(centralWidget);

    resizeEvent(nullptr);
}

MainWindow::~MainWindow()
{
    if (m_videoWindow) {
        m_videoWindow->close();
        delete m_videoWindow;
        m_videoWindow = nullptr;
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_videoThumbnail && event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            onThumbnailClicked();
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::onThumbnailClicked()
{
    if (m_videoWindow) {
        m_videoWindow->close();
        delete m_videoWindow;
        m_videoWindow = nullptr;
    }
    
    m_videoWindow = new VideoWindow();
    
    // 传递视频列表给播放器
    m_videoWindow->setVideoList(m_videos);
    
    m_videoWindow->show();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    if (!m_earthPix.isNull()) {
        QPixmap scaledEarth = m_earthPix.scaled(
            size(),
            Qt::KeepAspectRatioByExpanding,
            Qt::SmoothTransformation
            );
        m_backgroundLabel->setPixmap(scaledEarth);
        m_backgroundLabel->setGeometry(0, 0, width(), height());
    }

    if (!m_thumbnailPix.isNull()) {
        int maxW = width() * 0.7;
        int maxH = height() * 0.7;

        QSize thumbSize = m_thumbnailPix.size();
        thumbSize.scale(QSize(maxW, maxH), Qt::KeepAspectRatio);

        m_videoThumbnail->setPixmap(m_thumbnailPix.scaled(
            thumbSize,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
            ));
        m_videoThumbnail->setFixedSize(thumbSize);
    }
}

#include "mainwindow.h"
#include "videowindow.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QMouseEvent>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_videoWindow(nullptr)
{
    // 1. 基础设置
    setWindowTitle("移动端主页面");
    setMinimumSize(320, 480);
    setMaximumSize(1080, 1920);

    // 2. 加载图片资源
    m_earthPix = QPixmap(":/resources/earth.png");
    m_thumbnailPix = QPixmap(":/resources/video_thumbnail.png");

    // 3. 背景标签
    m_backgroundLabel = new QLabel(this);
    m_backgroundLabel->setAlignment(Qt::AlignCenter);
    m_backgroundLabel->setScaledContents(false);

    // 4. 视频缩略图
    m_videoThumbnail = new QLabel(this);
    m_videoThumbnail->setAlignment(Qt::AlignCenter);
    m_videoThumbnail->setStyleSheet(
        "border: 2px solid white; "
        "border-radius: 8px; "
        "cursor: pointer;"
        );
    m_videoThumbnail->setScaledContents(false);
    m_videoThumbnail->installEventFilter(this);

    // 5. 布局管理（调整上下拉伸比例，让缩略图上移）
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    // 上方空白区域小于下方（比例25:45），使缩略图上移
    mainLayout->addStretch(25);
    // 保持水平居中
    mainLayout->addWidget(m_videoThumbnail, 0, Qt::AlignHCenter);
    mainLayout->addStretch(45);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    setCentralWidget(centralWidget);

    resizeEvent(nullptr);
}

// 以下部分代码与之前一致（析构函数、事件过滤器、点击事件、尺寸调整）
MainWindow::~MainWindow()
{
    if (m_videoWindow) {
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
    }
    m_videoWindow = new VideoWindow();
    m_videoWindow->show();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (event == nullptr) return;

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
        // 保持70%窗口大小的设置
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

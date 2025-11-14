#include "mainwindow.h"
#include "home/homepage.h"
#include "player/videoplayer.h"
#include "friends/friendspage.h"
#include "capture/recordpage.h"
#include "chat/chatpage.h"
#include "profile/profilepage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Antipode: DayDreamNight");
    setGeometry(100, 100, 1024, 768);
    
    setupUI();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    // Create central widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // Create stacked widget for pages
    stackedWidget = new QStackedWidget(this);
    
    // Add pages
    stackedWidget->addWidget(new HomePage(this));           // 0
    stackedWidget->addWidget(new VideoPlayer(this));        // 1
    stackedWidget->addWidget(new FriendsPage(this));        // 2
    stackedWidget->addWidget(new RecordPage(this));         // 3
    stackedWidget->addWidget(new ChatPage(this));           // 4
    stackedWidget->addWidget(new ProfilePage(this));        // 5
    
    // Create main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(stackedWidget);
    
    createBottomNavigation();
}

void MainWindow::createBottomNavigation()
{
    // TODO: Create bottom navigation bar with 5 buttons
    // TODO: Connect buttons to page switching
}

void MainWindow::onHomeClicked()
{
    stackedWidget->setCurrentIndex(0);
}

void MainWindow::onFriendsClicked()
{
    stackedWidget->setCurrentIndex(2);
}

void MainWindow::onRecordClicked()
{
    stackedWidget->setCurrentIndex(3);
}

void MainWindow::onChatClicked()
{
    stackedWidget->setCurrentIndex(4);
}

void MainWindow::onProfileClicked()
{
    stackedWidget->setCurrentIndex(5);
}

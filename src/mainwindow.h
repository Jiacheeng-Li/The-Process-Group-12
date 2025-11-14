#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QStackedWidget *stackedWidget;
    
    void setupUI();
    void createBottomNavigation();
    
    // Slots for page switching
    void onHomeClicked();
    void onFriendsClicked();
    void onRecordClicked();
    void onChatClicked();
    void onProfileClicked();
};

#endif // MAINWINDOW_H

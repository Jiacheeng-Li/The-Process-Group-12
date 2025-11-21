#ifndef POPUPPANEL_H
#define POPUPPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

class PopupPanel : public QWidget
{
    Q_OBJECT
public:
    explicit PopupPanel(QWidget *parent = nullptr);

    void setTitle(const QString &text);
    void setContent(QWidget *content);

    void showPanel();
    void hidePanel();

protected:
    void leaveEvent(QEvent *event) override;

private:
    QWidget *box;
    QLabel *title;
    QVBoxLayout *layout;
    QWidget *contentWidget = nullptr;
};

#endif // POPUPPANEL_H

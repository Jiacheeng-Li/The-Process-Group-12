#include "popuppanel.h"
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QDebug>

PopupPanel::PopupPanel(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Popup);
    setAttribute(Qt::WA_StyledBackground);

    box = new QWidget(this);
    box->setStyleSheet(
        "background:white;"
        "border-radius:12px;"
        "border:2px solid #888;"
        );

    layout = new QVBoxLayout(box);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(10);

    title = new QLabel("");
    title->setStyleSheet("font-size:18px;font-weight:600;color:#333;");
    layout->addWidget(title);

    resize(320, 260);
    box->setGeometry(0,0, width(), height());
}

void PopupPanel::setTitle(const QString &text)
{
    title->setText(text);
}

void PopupPanel::setContent(QWidget *content)
{
    if (contentWidget)
        layout->removeWidget(contentWidget);

    contentWidget = content;
    layout->addWidget(contentWidget);
}

void PopupPanel::showPanel()
{
    QPoint globalPos = parentWidget()->mapToGlobal(
        QPoint(parentWidget()->width()/2 - width()/2,
               110)
        );
    move(globalPos);
    show();
    raise();
}

void PopupPanel::hidePanel()
{
    hide();
}

void PopupPanel::leaveEvent(QEvent *)
{
    hide();
}

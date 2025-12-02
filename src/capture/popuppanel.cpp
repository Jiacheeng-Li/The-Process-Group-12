#include "popuppanel.h"
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QDebug>
#include <QtGlobal>

PopupPanel::PopupPanel(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Popup);
    setAttribute(Qt::WA_StyledBackground);

    box = new QWidget(this);
    box->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(2,8,20,0.98), stop:1 rgba(13,13,13,0.98));"
        "border-radius: 16px;"
        "border: 3px solid #6CADFF;"
        );

    layout = new QVBoxLayout(box);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(12);

    title = new QLabel("");
    title->setStyleSheet(
        "font-size: 20px;"
        "font-weight: 700;"
        "color: white;"
        "padding-bottom: 8px;"
        "border-bottom: 2px solid #6CADFF;"
    );
    layout->addWidget(title);

    resize(400, 500);  // Increase panel size to ensure content is fully displayed
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
    
    // Auto-adjust size based on content, but set min and max sizes
    contentWidget->adjustSize();
    int contentHeight = contentWidget->sizeHint().height();
    int minHeight = 300;
    int maxHeight = 600;
    int newHeight = qBound(minHeight, contentHeight + 100, maxHeight);  // 100 is space for title and margins
    resize(width(), newHeight);
    box->setGeometry(0, 0, width(), height());
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

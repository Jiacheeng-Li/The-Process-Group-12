// for test in page 3
//zyx test_main.cpp
//    ______
//   /_  __/___  ____ ___  ___  ____
//    / / / __ \/ __ `__ \/ _ \/ __ \
//   / / / /_/ / / / / / /  __/ /_/ /
//  /_/  \____/_/ /_/ /_/\___/\____/
//         Record/Publish Test Only
//

#include "recordpage.h"
#include "publishpage.h"
#include <QApplication>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);


    QStackedWidget *stacked = new QStackedWidget;


    RecordPage *recordPage = new RecordPage;
    stacked->addWidget(recordPage);

    //  新增：主窗口
    QWidget window;
    QVBoxLayout *layout = new QVBoxLayout(&window);
    layout->addWidget(stacked);
    window.setLayout(layout);
    window.setWindowTitle("Record/Publish Test");
    window.resize(900, 700);
    window.show();

    //  新增：录制完成信号 → 切换到发布页
    QObject::connect(recordPage, &RecordPage::recordingFinished, [&](const QString &path) {
        PublishPage *pubPage = new PublishPage(path);
        stacked->addWidget(pubPage);
        stacked->setCurrentWidget(pubPage);

        QObject::connect(pubPage, &PublishPage::backRequested, [&]() {
            stacked->setCurrentWidget(recordPage);
        });
    });

    return app.exec();
}

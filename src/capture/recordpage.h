#ifndef RECORDPAGE_H
#define RECORDPAGE_H

#include <QWidget>
#include <QCamera>
#include <QMediaRecorder>

class RecordPage : public QWidget
{
    Q_OBJECT
public:
    explicit RecordPage(QWidget *parent = nullptr);

private:
    QCamera *camera;
    QMediaRecorder *mediaRecorder;
    
    // TODO: Add recording controls
    // TODO: Add preview functionality
    // TODO: Add upload logic
};

#endif // RECORDPAGE_H

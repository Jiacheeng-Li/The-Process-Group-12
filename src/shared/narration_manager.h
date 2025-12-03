#ifndef NARRATION_MANAGER_H
#define NARRATION_MANAGER_H

#include <QObject>
#include <QString>
#include "../home/app_settings.h"

class QTextToSpeech;

class NarrationManager : public QObject {
    Q_OBJECT
public:
    static NarrationManager &instance();
    
    void initialize(QObject *parent = nullptr);
    void setEnabled(bool enabled);
    void setLanguage(AppLanguage language);
    void narrate(const QString &text);
    void narrate(const QString &zh, const QString &en);
    
    bool isEnabled() const { return enabled_; }

private:
    NarrationManager();
    ~NarrationManager();
    
    QTextToSpeech *speech_;
    bool enabled_;
    AppLanguage currentLanguage_;
};

#endif // NARRATION_MANAGER_H


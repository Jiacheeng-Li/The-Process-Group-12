#ifndef LANGUAGE_MANAGER_H
#define LANGUAGE_MANAGER_H

#include <QObject>
#include "../home/app_settings.h"

class LanguageManager : public QObject {
    Q_OBJECT
public:
    static LanguageManager &instance();

    AppLanguage currentLanguage() const;
    void setLanguage(AppLanguage lang);

    QString pick(const QString &zh, const QString &en) const;

signals:
    void languageChanged(AppLanguage newLanguage);

private:
    LanguageManager();
    AppLanguage currentLanguage_;
};

#endif // LANGUAGE_MANAGER_H


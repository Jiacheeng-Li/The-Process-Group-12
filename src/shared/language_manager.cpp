#include "language_manager.h"

LanguageManager &LanguageManager::instance() {
    static LanguageManager instance;
    return instance;
}

LanguageManager::LanguageManager()
    : QObject(nullptr),
      currentLanguage_(AppLanguage::English) {}

AppLanguage LanguageManager::currentLanguage() const {
    return currentLanguage_;
}

void LanguageManager::setLanguage(AppLanguage lang) {
    if (lang == currentLanguage_) {
        return;
    }
    currentLanguage_ = lang;
    emit languageChanged(currentLanguage_);
}

QString LanguageManager::pick(const QString &zh, const QString &en) const {
    return currentLanguage_ == AppLanguage::Chinese ? zh : en;
}


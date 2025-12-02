#include "narration_manager.h"
#include <QTextToSpeech>
#include <QLocale>

NarrationManager &NarrationManager::instance() {
    static NarrationManager instance;
    return instance;
}

NarrationManager::NarrationManager()
    : QObject(nullptr),
      speech_(nullptr),
      enabled_(false),
      currentLanguage_(AppLanguage::Chinese) {
}

NarrationManager::~NarrationManager() {
    if (speech_) {
        delete speech_;
        speech_ = nullptr;
    }
}

void NarrationManager::initialize(QObject *parent) {
    if (speech_) {
        return; // Already initialized
    }
    
    try {
        speech_ = new QTextToSpeech(parent);
        speech_->setLocale(QLocale(QLocale::Chinese, QLocale::China));
    } catch (...) {
        speech_ = nullptr;
    }
}

void NarrationManager::setEnabled(bool enabled) {
    enabled_ = enabled;
}

void NarrationManager::setLanguage(AppLanguage language) {
    currentLanguage_ = language;
    if (speech_) {
        QLocale locale = language == AppLanguage::Chinese 
                         ? QLocale(QLocale::Chinese, QLocale::China)
                         : QLocale(QLocale::English, QLocale::UnitedStates);
        speech_->setLocale(locale);
    }
}

void NarrationManager::narrate(const QString &text) {
    if (!enabled_ || !speech_ || text.trimmed().isEmpty()) {
        return;
    }
    speech_->say(text.trimmed());
}

void NarrationManager::narrate(const QString &zh, const QString &en) {
    QString text = currentLanguage_ == AppLanguage::Chinese ? zh : en;
    narrate(text);
}


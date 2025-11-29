#ifndef APP_SETTINGS_H
#define APP_SETTINGS_H

#include <QString>

enum class AppLanguage {
    Chinese,
    English
};

struct AccessibilityPreferences {
    bool narrationEnabled = false;
    bool subtitlesEnabled = true;
    bool keyboardNavigationEnabled = true;
    bool colorBlindPaletteEnabled = false;
    AppLanguage interfaceLanguage = AppLanguage::Chinese;
    QString subtitleLanguage = QStringLiteral("zh");
    // 视频显示参数 (0.0-2.0, 1.0为默认值)
    double brightness = 1.0;
    double contrast = 1.0;
    double saturation = 1.0;
    // 播放倍速 (0.25-4.0, 1.0为默认值)
    double playbackRate = 1.0;
};

#endif // APP_SETTINGS_H


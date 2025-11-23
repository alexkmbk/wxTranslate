#pragma once

#include <filesystem>
#include <string>

struct Settings
{
    static const std::filesystem::path settingsFilePath;
    std::vector<std::wstring> favLangs;
    std::wstring sourceLang;
    std::wstring currentLangIn;
    std::wstring currentLangOut;

    void load();
    void save() const;
    void setInCurrentLang(const std::wstring& lang) {
        currentLangIn = lang;
	}
    void setOutCurrentLang(const std::wstring& lang) {
        currentLangOut = lang;
	}
};

extern Settings g_Settings;


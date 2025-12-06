#pragma once

#include <filesystem>
#include <string>

struct Settings
{
    static const std::filesystem::path settingsFilePath;
    std::vector<std::string> favLangs;
    std::string sourceLang;
    std::string currentLangIn;
    std::string currentLangOut;

    void load();
    void save() const;
    void setInCurrentLang(const std::string& lang) {
        currentLangIn = lang;
	}
    void setOutCurrentLang(const std::string& lang) {
        currentLangOut = lang;
	}
};

extern Settings g_Settings;


#include ".\lib\SimpleIni\SimpleIni.h"

#include "utils.h"
#include "settings.h"

namespace fs = std::filesystem;

std::vector<std::wstring> favLangs;

//const fs::path settingsFile = L"settings.ini";

const std::filesystem::path Settings::settingsFilePath = L"settings.ini";

Settings g_Settings;

void Settings::load() {
    
    if (fs::exists(settingsFilePath)) {
        CSimpleIniA ini;
        ini.SetUnicode();
        ini.LoadFile(settingsFilePath.c_str());
        favLangs = split(ini.GetValue("Translate", "FavoriteLanguages", ""), ',');
        currentLangIn = ini.GetValue("Translate", "CurrentLanguageIn", "");
        currentLangOut = ini.GetValue("Translate", "CurrentLanguageOut", "");
    }
    

    if (favLangs.empty()) {
        favLangs.push_back("en");
    }
    if (currentLangIn.empty()) {
        currentLangIn = "auto";
	}
    if (currentLangOut.empty() && favLangs.size() > 0) {
        currentLangOut = favLangs[0];
    }
}

void Settings::save() const {
    CSimpleIniA ini;
    ini.SetUnicode();
    ini.SetValue("Translate", "FavoriteLanguages", "");
    std::string favLangsStr;
    for (const auto& lang : favLangs) {
        if (!favLangsStr.empty()) {
            favLangsStr += ",";
        }
        favLangsStr += lang;
    }
    ini.SetValue("Translate", "FavoriteLanguages", favLangsStr.c_str());
    ini.SetValue("Translate", "CurrentLanguageIn", currentLangIn.c_str());
    ini.SetValue("Translate", "CurrentLanguageOut", currentLangOut.c_str());
    ini.SaveFile(settingsFilePath.c_str());
}
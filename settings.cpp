#include ".\lib\SimpleIni\SimpleIni.h"

#include "utils.h"
#include "settings.h"

namespace fs = std::filesystem;

std::vector<std::wstring> favLangs;

//const fs::path settingsFile = L"settings.ini";

const std::filesystem::path Settings::settingsFilePath = L"settings.ini";

Settings g_Settings;

void Settings::load() {
    
    if (!fs::exists(settingsFilePath)) {
        return;
    }
    CSimpleIniW ini;
    ini.SetUnicode();
    ini.LoadFile(settingsFilePath.c_str());
    favLangs = split(ini.GetValue(L"Translate", L"FavoriteLanguages", L""), L',');
    currentLangIn = ini.GetValue(L"Translate", L"CurrentLanguageIn", L"");
    currentLangOut = ini.GetValue(L"Translate", L"CurrentLanguageOut", L"");
    if (currentLangIn.empty() && !favLangs.empty()) {
        currentLangIn = favLangs[0];
	}
    if (currentLangOut.empty() && favLangs.size() > 1) {
        currentLangOut = favLangs[1];
    }
}

void Settings::save() const {
    CSimpleIniW ini;
    ini.SetUnicode();
    ini.SetValue(L"Translate", L"FavoriteLanguages", L"");
    std::wstring favLangsStr;
    for (const auto& lang : favLangs) {
        if (!favLangsStr.empty()) {
            favLangsStr += L",";
        }
        favLangsStr += lang;
    }
    ini.SetValue(L"Translate", L"FavoriteLanguages", favLangsStr.c_str());
    ini.SetValue(L"Translate", L"CurrentLanguageIn", currentLangIn.c_str());
    ini.SetValue(L"Translate", L"CurrentLanguageOut", currentLangOut.c_str());
    ini.SaveFile(settingsFilePath.c_str());
}
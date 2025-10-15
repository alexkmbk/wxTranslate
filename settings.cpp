#include ".\lib\SimpleIni\SimpleIni.h"

#include "utils.h"
#include "settings.h"

namespace fs = std::filesystem;

std::vector<std::wstring> favLangs;
fs::path settingsFile = L"settings.ini";

void readSettings() {
    
    if (!fs::exists(settingsFile)) {
        return;
    }
    CSimpleIniW ini;
    ini.SetUnicode();
    ini.LoadFile(settingsFile.c_str());
    favLangs = split(ini.GetValue(L"Translate", L"FavoriteLanguages", L""), L',');
}
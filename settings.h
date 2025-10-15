#pragma once

#include <filesystem>

extern std::filesystem::path settingsFile;
extern std::vector<std::wstring> favLangs;
void readSettings();


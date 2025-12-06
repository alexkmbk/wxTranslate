#pragma once

#include "lang_panel.h"

enum class Lang {
    Russian,
    English,
    Unknown
};

//Lang DetectLang(const std::string& text);
void translate(wxFrame* frame, wxString text, std::function<void(const std::string& translated, const std::string& sourceLanguage, const std::string& error)> callback);
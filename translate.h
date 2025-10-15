#pragma once

enum class Lang {
    Russian,
    English,
    Unknown
};

Lang DetectLang(const std::string& text);
void translate(wxFrame* frame, wxTextCtrl* textCtrl, wxString text);
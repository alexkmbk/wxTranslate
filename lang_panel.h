#pragma once
#include <wx/wx.h>
#include <wx/wrapsizer.h>
#include <wx/tglbtn.h> // For wxToggleButton

#include <vector>
#include <string>
#include <functional>
#include <map>


class LangPanel : public wxPanel
{
public:
    using LangCallback = std::function<void(const std::string&)>;

    LangPanel(wxWindow* parent,
        const std::map<std::string, std::string>& favLangs,
        LangCallback onLangClick, const std::string& initiallyPressedLang);
    void UpdateButtonLabel(const wxString& langCode, const wxString& newLabel);
    void PressButtonByLangCode(const std::string& langCode);

private:
    std::vector<std::string> langs_; // stores language codes
    std::vector<wxToggleButton*> buttons_;
    LangCallback callback_;
    int pressedIndex_;

    void OnButtonClicked(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

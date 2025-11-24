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
    using LangCallback = std::function<void(const std::wstring&)>;

    LangPanel(wxWindow* parent,
        const std::map<std::wstring, std::wstring>& favLangs,
        LangCallback onLangClick, const std::wstring& initiallyPressedLang);
    void UpdateButtonLabel(const wxString& langCode, const wxString& newLabel);
private:
    std::vector<std::wstring> langs_; // stores language codes
    std::vector<wxToggleButton*> buttons_;
    LangCallback callback_;
    int pressedIndex_;

    void OnButtonClicked(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

// lang_panel.cpp
#include "lang_panel.h"
wxBEGIN_EVENT_TABLE(LangPanel, wxPanel)
EVT_TOGGLEBUTTON(wxID_ANY, LangPanel::OnButtonClicked)
wxEND_EVENT_TABLE()

LangPanel::LangPanel(wxWindow* parent,
    const std::map<std::string, std::string>& favLangs,
    LangCallback onLangClick,
    const std::string& initiallyPressedLang)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL)
    , callback_(onLangClick)
    , pressedIndex_(-1)
{
    auto* sizer = new wxWrapSizer(wxHORIZONTAL);
    SetSizer(sizer);

    int buttonHeight = 0;
    size_t i = 0;
    for (const auto& [langCode, buttonLabel] : favLangs)
    {
        langs_.push_back(langCode);
        wxString label = wxString::FromUTF8(std::string(buttonLabel.begin(), buttonLabel.end()));
        auto* btn = new wxToggleButton(this, static_cast<int>(1000 + i), label);
        if (langCode == initiallyPressedLang) {
            btn->SetValue(true);
            pressedIndex_ = i;
        }

        sizer->Add(btn, 0);

        if (i == 0)
            buttonHeight = btn->GetEffectiveMinSize().y;
        buttons_.push_back(btn);
        ++i;
    }

    //if (!langs_.empty())
    //{
    //    int panelHeight = buttonHeight + 4;
    //    SetMinSize(wxSize(-1, panelHeight));
    //    SetMaxSize(wxSize(-1, panelHeight));
    //    SetSizeHints(-1, panelHeight);
    //}
    //else
    //{
    //    SetMinSize(wxSize(0, 0));
    //}
    // Fit(); // optional
}



void LangPanel::OnButtonClicked(wxCommandEvent& event)
{
    int id = event.GetId();
    size_t index = id - 1000;

    if (index >= langs_.size())
        return;

    // Unpress previous button if any
    if (pressedIndex_ != -1 && pressedIndex_ != index && pressedIndex_ < buttons_.size()) {
        buttons_[pressedIndex_]->SetValue(false);
    }

    // Press current button
    buttons_[index]->SetValue(true);
    pressedIndex_ = index;

    if (callback_)
        callback_(langs_[index]);
}

void LangPanel::UpdateButtonLabel(const wxString& langCode, const wxString& newLabel)
{
    auto it = std::find_if(langs_.begin(), langs_.end(),
        [&](const std::string& code) {
            return wxString::FromUTF8(std::string(code.begin(), code.end())) == langCode;
        });
    if (it == langs_.end())
        return;

    size_t index = std::distance(langs_.begin(), it);
    if (index >= buttons_.size())
        return;

    buttons_[index]->SetLabel(newLabel);
}

void LangPanel::PressButtonByLangCode(const std::string& langCode)
{
    auto it = std::find(langs_.begin(), langs_.end(), langCode);
    if (it == langs_.end())
        return;

    size_t index = std::distance(langs_.begin(), it);
    if (index >= buttons_.size())
        return;

    // Unpress previous button if any
    if (pressedIndex_ != -1 && pressedIndex_ != index && pressedIndex_ < buttons_.size()) {
        buttons_[pressedIndex_]->SetValue(false);
    }

    // Press current button
    buttons_[index]->SetValue(true);
    pressedIndex_ = static_cast<int>(index);
}

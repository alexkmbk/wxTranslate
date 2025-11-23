// lang_panel.cpp
#include "lang_panel.h"
wxBEGIN_EVENT_TABLE(LangPanel, wxPanel)
EVT_TOGGLEBUTTON(wxID_ANY, LangPanel::OnButtonClicked)
wxEND_EVENT_TABLE()

LangPanel::LangPanel(wxWindow* parent,
    const std::vector<std::wstring>& favLangs,
    LangCallback onLangClick,
    const std::wstring& initiallyPressedLang)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL)
    , langs_(favLangs)
    , callback_(onLangClick)
    , pressedIndex_(-1)
{
    auto* sizer = new wxWrapSizer(wxHORIZONTAL);
    SetSizer(sizer);

    int buttonHeight = 0;

    for (size_t i = 0; i < langs_.size(); ++i)
    {
        wxString label = wxString::FromUTF8(std::string(langs_[i].begin(), langs_[i].end()));
        auto* btn = new wxToggleButton(this, static_cast<int>(1000 + i), label);

        // Set pressed state if matches initiallyPressedLang
        if (langs_[i] == initiallyPressedLang) {
            btn->SetValue(true);
            pressedIndex_ = i;
        }

        sizer->Add(btn, wxSizerFlags().Expand());

        if (i == 0)
            buttonHeight = btn->GetEffectiveMinSize().y;
        // Store pointer for later toggling
        buttons_.push_back(btn);
    }

    if (!langs_.empty())
    {
        int panelHeight = buttonHeight + 4;
        SetMinSize(wxSize(-1, panelHeight));
        SetMaxSize(wxSize(-1, panelHeight));
        SetSizeHints(-1, panelHeight);
    }
    else
    {
        SetMinSize(wxSize(0, 0));
    }
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

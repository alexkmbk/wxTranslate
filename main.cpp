#include <wx/wx.h>
#include <wx/webrequest.h>
#include <wx/timer.h>
#include <wx/textctrl.h>
#include <wx/clipbrd.h>

#include <sstream>
#include <iomanip>
#include <vector>
#include <windows.h>
#include <algorithm> 

#include "translate.h"
#include "taskbar.h"
#include "selected_text.h"
#include "utils.h"
#include "settings.h"
#include "lang_panel.h"

#include <debugapi.h>

using namespace std;

//fs::path currentDir = std::filesystem::current_path();

enum {
    ID_WebRequest = wxID_HIGHEST + 1,
    ID_InputDebounceTimer,
    ID_HOTKEY_SHOW = 10001,
    ID_HOTKEY_TRANSLATE_SELECTED = 10002
};

class MyFrame : public wxFrame
{
private:
    bool m_lastActionWasPaste = false;
    wxDECLARE_EVENT_TABLE();

public:
    wxTextCtrl* inputText;
    wxTextCtrl* outputText;
    LangPanel* leftPanel;
    LangPanel* rightPanel;
    wxTimer* debounceTimer;
    wxString lastInputText;
    MyTaskBarIcon* trayIcon = nullptr; // Now the compiler knows about MyTaskBarIcon

    void afterTranslateCallback(const std::string& translated, const std::string& sourceLanguage, const std::string& error)
    {
        if (!error.empty()) {
            wxMessageBox(L"Ошибка перевода: " + wxString(error), L"Ошибка", wxOK | wxICON_ERROR);
            return;
        }

		// Update left panel if input language is auto
		if (g_Settings.currentLangIn == "auto") {
			if (sourceLanguage.empty()) {
				leftPanel->UpdateButtonLabel("auto", "Auto (unknown)");
			}
			else
				leftPanel->UpdateButtonLabel("auto", wxString::Format("Auto (%s)", wxString::FromUTF8(sourceLanguage)));
		}

        if (sourceLanguage == g_Settings.currentLangOut) {
          for (auto& langPair : g_Settings.favLangs) {
              if (langPair == sourceLanguage) {
                  continue;
              }
              g_Settings.setOutCurrentLang(langPair);
			  rightPanel->PressButtonByLangCode(langPair);
              translate(this, inputText->GetValue(), [this](const std::string& t,
                  const std::string& sl,
                  const std::string& err)
                  {
                      afterTranslateCallback(t, sl, err);
                  });
              break;
		  }
		}
        outputText->SetValue(wxString::FromUTF8(translated.c_str()));
    }

    MyFrame()
        : wxFrame(nullptr, wxID_ANY, "wxTranslate", wxDefaultPosition, wxSize(800, 400))
    {
        // Кнопка копирования
        wxButton* copyButton = new wxButton(this, wxID_ANY, "Copy");

		wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);
		// Set larger font for input and output text controls
		wxFont largeFont{ 14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL };
		
        // Input

        // Left lang panel

        auto* leftPanelSizer = new wxBoxSizer(wxHORIZONTAL);

        // Convert favLangs to std::map<std::wstring, std::wstring> if needed
        std::map<std::string, std::string> favLangsMap{ {"auto", "Auto"} };
        
        for (const auto& langCode : g_Settings.favLangs) {
            favLangsMap[langCode] = langCode;
        }
        
        leftPanel = new LangPanel(this, favLangsMap, [this](const std::string& lang) {
            if (lang != g_Settings.currentLangOut) {
                g_Settings.setInCurrentLang(lang);
                translate(this, inputText->GetValue(), [this](const std::string& t,
                    const std::string& sl,
                    const std::string& err)
                    {
                        afterTranslateCallback(t, sl, err);
                    });
			}
            }, g_Settings.currentLangIn);

        leftPanelSizer->Add(leftPanel, 0, wxEXPAND | wxALL, 0);


        wxBoxSizer* inputSizer = new wxBoxSizer(wxVERTICAL);
        inputSizer->Add(leftPanelSizer, 0, wxEXPAND | wxALL, 5);
        inputText = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
			wxTE_MULTILINE);
		inputText->SetFont(largeFont);
        inputSizer->Add(inputText, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);    

		//mainSizer->Add(inputText, 1, wxEXPAND | wxALL, 5);

        // Создаём вертикальный сайзер для outputText и кнопки
        wxBoxSizer* outputSizer = new wxBoxSizer(wxVERTICAL);

        favLangsMap.erase("auto");

        rightPanel = new LangPanel(this, favLangsMap, [this](const std::string& lang) {
            if (lang != g_Settings.currentLangOut) {
                g_Settings.setOutCurrentLang(lang);
                translate(this, inputText->GetValue(), [this](const std::string& t,
                    const std::string& sl,
                    const std::string& err)
                    {
                        afterTranslateCallback(t, sl, err);
                    });
            }
            }, g_Settings.currentLangOut);
        auto* rightPanelSizer = new wxBoxSizer(wxHORIZONTAL);
        rightPanelSizer->Add(rightPanel, 0, wxEXPAND | wxRIGHT, 0);
        rightPanelSizer->AddStretchSpacer();
        rightPanelSizer->Add(copyButton, 0);
        outputSizer->Add(rightPanelSizer, 0, wxEXPAND | wxALL, 5);

        // Поле вывода
        outputText = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
            wxTE_MULTILINE);
        outputText->SetFont(largeFont);
        outputSizer->Add(outputText, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

        
        mainSizer->Add(inputSizer, 1, wxEXPAND);
        mainSizer->Add(outputSizer, 1, wxEXPAND);

        SetSizer(mainSizer);
        Layout();

        inputText->Bind(wxEVT_TEXT_PASTE, &MyFrame::OnInputTextPaste, this);
        inputText->Bind(wxEVT_TEXT, &MyFrame::OnInputTextChanged, this);
        
        // Привязываем обработчик к кнопке копирования
        copyButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
            if (wxTheClipboard->Open()) {
                wxTheClipboard->SetData(new wxTextDataObject(outputText->GetValue()));
                wxTheClipboard->Close();
            }
            Hide(); // Скрыть окно после копирования
            });

        Bind(wxEVT_CHAR_HOOK, &MyFrame::OnCharHook, this);

        debounceTimer = new wxTimer(this, ID_InputDebounceTimer);
        Bind(wxEVT_TIMER, &MyFrame::OnDebounceTimer, this, ID_InputDebounceTimer);

        // Создаём иконку в трее
        wxIcon icon = wxArtProvider::GetIcon(wxART_INFORMATION, wxART_OTHER, wxSize(16, 16));
        trayIcon = new MyTaskBarIcon(this); // Initialize trayIcon with an instance of MyTaskBarIcon
        trayIcon->SetIcon(icon, "wxTranslate");

        // Сразу сворачиваем в трей при запуске
        Hide();
    }

    // Обработка закрытия окна — сворачиваем в трей, не завершаем приложение
    void OnClose(wxCloseEvent& event) 
    {
        Hide();
        if (trayIcon)
            trayIcon->ShowBalloon("wxTranslate", "Приложение свернуто в трей", 1000, wxICON_INFORMATION);
        event.Veto(); // Не уничтожаем окно!
    }

    // Обработка хоткея
    WXLRESULT MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam) override
    {
        if (nMsg == WM_HOTKEY && wParam == ID_HOTKEY_SHOW)
        {
            if (!IsShown() || IsIconized())
                Show();
            if (IsIconized())
                Iconize(false);
            Raise();
            inputText->SetFocus();
            return 0;
        }
        else if (nMsg == WM_HOTKEY && wParam == ID_HOTKEY_TRANSLATE_SELECTED)
        {            
            std::wstring selected = GetSelectedText();
            m_lastActionWasPaste = true;
            inputText->SetValue(selected.c_str());
            translate(this, selected, [this](const std::string& t,
                const std::string& sl,
                const std::string& err)
                {
                    afterTranslateCallback(t, sl, err);
                });
            if (!IsShown() || IsIconized())
                Show();
            if (IsIconized())
                Iconize(false);
            Raise();
            inputText->SetFocus();

            return 0;
		}
        return wxFrame::MSWWindowProc(nMsg, wParam, lParam);
    }

    void OnInputTextChanged(wxCommandEvent& event)
    {
        if (m_lastActionWasPaste) {
            m_lastActionWasPaste = false;
            return;
        }
        lastInputText = inputText->GetValue();
        if (lastInputText.empty()) {
            outputText->SetValue("");
            return;
        }
        if (!debounceTimer->IsRunning()) {
            debounceTimer->Start(1000, true);
        }        
    }

    void OnInputTextPaste(wxCommandEvent& event)
    {
		m_lastActionWasPaste = true;
        if (wxTheClipboard->Open()) {
            if (wxTheClipboard->IsSupported(wxDF_TEXT)) {
                wxTextDataObject data;
                wxTheClipboard->GetData(data);

                lastInputText = data.GetText();
            }
            wxTheClipboard->Close();
        }

        if (lastInputText.empty()) {
            outputText->SetValue("");
        }
        else {
            translate(this, lastInputText, [this](const std::string& t,
                const std::string& sl,
                const std::string& err)
                {
                    afterTranslateCallback(t, sl, err);
                });
        }
        event.Skip();
    }

    void OnDebounceTimer(wxTimerEvent& event)
    {
        wxString text = lastInputText;
        if (text.IsEmpty()) {
            outputText->SetValue("");
            return;
        }

        lastInputText = inputText->GetValue();
        if (lastInputText.empty()) {
            outputText->SetValue("");
        }
        else {
            translate(this, lastInputText, [this](const std::string& t,
                const std::string& sl,
                const std::string& err)
                {
                    afterTranslateCallback(t, sl, err);
                });
        }
    }

    void OnCharHook(wxKeyEvent& event)
    {
        if (event.GetKeyCode() == WXK_ESCAPE) {
            Hide();
            return;
        }
        event.Skip();
    }

    ~MyFrame() override
    {
        if (trayIcon) {
            trayIcon->RemoveIcon();
            delete trayIcon;
        }
    }

};

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_CLOSE(MyFrame::OnClose)    
wxEND_EVENT_TABLE()

class MyApp : public wxApp
{
public:
    MyFrame* frame = nullptr;

    bool OnInit() override
    {
		g_Settings.load();
        frame = new MyFrame();

        frame->SetBackgroundColour(wxColour(240, 240, 240));

        // frame->Show(); // Не показываем окно при запуске, оно сразу в трее

        // Регистрируем глобальный хоткей Ctrl+alt+T
        HWND hwnd = (HWND)frame->GetHWND();
        if (!RegisterHotKey(hwnd, ID_HOTKEY_SHOW, MOD_CONTROL | MOD_ALT, 'T')) {
            wxLogError("Не удалось зарегистрировать хоткей Ctrl+Alt+T");
        }

        // Регистрируем глобальный хоткей Ctrl+alt+E
        if (!RegisterHotKey(hwnd, ID_HOTKEY_TRANSLATE_SELECTED, MOD_CONTROL | MOD_ALT, 'E')) {
            wxLogError("Не удалось зарегистрировать хоткей Ctrl+Alt+E");
        }

        return true;
    }

    int OnExit() override
    {
        // Снимаем регистрацию хоткея
        if (frame) {
            HWND hwnd = (HWND)frame->GetHWND();
            UnregisterHotKey(hwnd, ID_HOTKEY_SHOW);
            UnregisterHotKey(hwnd, ID_HOTKEY_TRANSLATE_SELECTED);
        }
        g_Settings.save();
        return 0;
    }
};

wxIMPLEMENT_APP(MyApp);


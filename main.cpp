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

#include <debugapi.h>

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
    wxTimer* debounceTimer;
    wxString lastInputText;
    MyTaskBarIcon* trayIcon = nullptr; // Now the compiler knows about MyTaskBarIcon

    MyFrame()
        : wxFrame(nullptr, wxID_ANY, "wxTranslate", wxDefaultPosition, wxSize(800, 400))
    {
        wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);

        inputText = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
            wxTE_MULTILINE | wxTE_RICH2);
        mainSizer->Add(inputText, 1, wxEXPAND | wxALL, 5);

        outputText = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
            wxTE_MULTILINE | wxTE_RICH2);
        mainSizer->Add(outputText, 1, wxEXPAND | wxALL, 5);

        SetSizer(mainSizer);
        Layout();

        inputText->Bind(wxEVT_TEXT_PASTE, &MyFrame::OnInputTextPaste, this);
        inputText->Bind(wxEVT_TEXT, &MyFrame::OnInputTextChanged, this);
        

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

    // Обработка двойного клика по иконке в трее — показываем окно
    void OnTaskBarLeftDClick(wxTaskBarIconEvent&)
    {
        Show();
        Raise();
    }

    // Обработка хоткея
    WXLRESULT MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam) override
    {
        if (nMsg == WM_HOTKEY && wParam == ID_HOTKEY_SHOW)
        {
            if (!IsShown())
                Show();
            Raise();
            return 0;
        }
        else if (nMsg == WM_HOTKEY && wParam == ID_HOTKEY_TRANSLATE_SELECTED)
        {
            //DWORD fgThread = GetWindowThreadProcessId(GetForegroundWindow(), nullptr);
            //DWORD myThread = GetCurrentThreadId();

            //AttachThreadInput(myThread, fgThread, TRUE);
            //HWND focused = GetFocus();

            //// Если у этого окна нет выделения — ищем среди потомков
            //if (focused) {
            //    LRESULT sel = SendMessage(focused, EM_GETSEL, 0, 0);
            //    DWORD start = LOWORD(sel);
            //    DWORD end = HIWORD(sel);
            //    if (start == end) {
            //        HWND child = GetFocusedOrSelectedChild(focused);
            //        if (child)
            //            focused = child;
            //    }
            //}

            //AttachThreadInput(myThread, fgThread, FALSE);

            //std::wstring selected;

            //if (focused) {

            //    // 1. Узнаем длину текста (WM_GETTEXTLENGTH)
            //    // SendMessage для кросс-процессной передачи текста
            //    int len = (int)SendMessageW(focused, WM_GETTEXTLENGTH, 0, 0);

            //    // 2. Выделяем буфер
            //    std::wstring buf(len + 1, L'\0');

            //    // Получаем границы выделения.
            //    // Для EM_GETSEL, начало (start) в LOWORD, конец (end) в HIWORD LRESULT.
            //    LRESULT sel = SendMessage(focused, EM_GETSEL, 0, 0);

            //    // Извлекаем start и end из LRESULT
            //    DWORD start = LOWORD(sel);
            //    DWORD end = HIWORD(sel);

            //    int charsCopied = (int)SendMessageW(focused, WM_GETTEXT, (WPARAM)len + 1, (LPARAM)&buf[0]);

            //    if (charsCopied > 0) {
            //        
            //        // Replace the problematic line with the following:
            //        end = std::min(static_cast<DWORD>(charsCopied), end);
            //        // Извлекаем подстроку
            //        if (start < end && end <= buf.size()) {
            //            selected = buf.substr(start, end - start);
            //        }
            //    }
            //}

            std::wstring selected = GetSelectedText();
            /*if (wxTheClipboard->Open()) {
                if (wxTheClipboard->IsSupported(wxDF_TEXT)) {
                    wxTextDataObject data;
                    wxTheClipboard->GetData(data);

                    selected = data.GetText();
                }
                wxTheClipboard->Close();
            }*/

            m_lastActionWasPaste = true;
            //std::wstring text = GetSelectedText();
            inputText->SetValue(selected.c_str());
            translate(this, outputText, selected);
            if (!IsShown())
                Show();
            Raise();

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
            debounceTimer->Start(2000, true);
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
            translate(this, outputText, lastInputText);
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
            translate(this, outputText, lastInputText);
        }
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
    EVT_TASKBAR_LEFT_DCLICK(MyFrame::OnTaskBarLeftDClick)
wxEND_EVENT_TABLE()

class MyApp : public wxApp
{
public:
    MyFrame* frame = nullptr;

    bool OnInit() override
    {
        frame = new MyFrame();
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
        return 0;
    }
};

wxIMPLEMENT_APP(MyApp);


#include <wx/sstream.h>
#include <wx/wx.h>
#include <wx/webrequest.h>
#include <wx/timer.h>

#include <sstream>
#include <iomanip>
#include <vector>
#include <windows.h>

#include "lang.h"
#include "taskbar.h"

enum {
    ID_WebRequest = wxID_HIGHEST + 1,
    ID_InputDebounceTimer,
    ID_HOTKEY_SHOW = 10001,
    ID_HOTKEY_TRANSLATE_SELECTED = 10002
};

class MyFrame : public wxFrame
{
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
            // 1. Получаем потоки (как у вас)
            DWORD fgThread = GetWindowThreadProcessId(GetForegroundWindow(), nullptr);
            DWORD myThread = GetCurrentThreadId();

            // 2. Подключаемся к чужому потоку (как у вас)
            AttachThreadInput(myThread, fgThread, TRUE);
            HWND focused = GetFocus();
            AttachThreadInput(myThread, fgThread, FALSE);

            if (focused) {

                // 1. Узнаем длину текста (WM_GETTEXTLENGTH)
                // SendMessage для кросс-процессной передачи текста
                int len = (int)SendMessageW(focused, WM_GETTEXTLENGTH, 0, 0);

                // 2. Выделяем буфер
                std::wstring buf(len + 1, L'\0');

                // 3. Получаем текст (WM_GETTEXT). Система выполнит маршаллинг буфера.
                SendMessageW(focused, WM_GETTEXT, (WPARAM)len + 1, (LPARAM)&buf[0]);

                // Получаем границы выделения.
                // Для EM_GETSEL, начало (start) в LOWORD, конец (end) в HIWORD LRESULT.
                LRESULT sel = SendMessage(focused, EM_GETSEL, 0, 0);

                // Извлекаем start и end из LRESULT
                DWORD start = LOWORD(sel);
                DWORD end = HIWORD(sel);

                // Проверяем корректность и извлекаем подстроку
                if (start < end && end <= buf.size()) {
                    std::wstring selected = buf.substr(start, end - start);
                    // ... (Ваша логика с wxWidgets)
                    inputText->SetValue(selected.c_str());
                }
            }
            if (!IsShown())
                Show();
            Raise();

            return 0;
		}
        return wxFrame::MSWWindowProc(nMsg, wParam, lParam);
    }

    static std::string UrlEncode(const std::string& value) {
        std::ostringstream escaped;
        escaped.fill('0');
        escaped << std::hex;
        for (char c : value) {
            if (isalnum((unsigned char)c) || c == '-' || c == '_' || c == '.' || c == '~') {
                escaped << c;
            } else {
                escaped << '%' << std::setw(2) << int((unsigned char)c);
            }
        }
        return escaped.str();
    }

    void OnInputTextChanged(wxCommandEvent& event)
    {
        lastInputText = inputText->GetValue();
        debounceTimer->Start(1000, true);
    }

    void OnDebounceTimer(wxTimerEvent& event)
    {
        wxString text = lastInputText;
        if (text.IsEmpty()) {
            outputText->SetValue("");
            return;
        }

        std::string query{ text.ToUTF8() };
        std::string encoded = UrlEncode(query);

        wxString url;
        if (DetectLang(query) == Lang::English) {
            url = wxString::Format(
                "https://translate.google.com/m?sl=en&tl=ru&hl=ru&q=%s",
                wxString(encoded)
            );
        }
        else{
            url = wxString::Format(
                "https://translate.google.com/m?sl=ru&tl=en&hl=ru&q=%s",
                wxString(encoded)
            );
        }

        wxWebRequest req = wxWebSession::GetDefault().CreateRequest(this, url);

        Bind(wxEVT_WEBREQUEST_STATE, [this](wxWebRequestEvent& evt) {
			if (evt.GetState() == wxWebRequest::State_Completed) {
				wxString response = evt.GetResponse().AsString();

                // Парсим предпоследний <div> с конца
				std::string html = std::string(response.ToUTF8());
				std::string result;
				size_t end = html.rfind("</div>");
				if (end != std::string::npos) {
					// Найти второй с конца </div>
					size_t prev_end = html.rfind("</div>", end - 1);
					if (prev_end != std::string::npos) {
						// Найти начало <div ...> перед prev_end
						size_t div_start = html.rfind("<div", prev_end);
						if (div_start != std::string::npos) {
							size_t content_start = html.find('>', div_start);
							if (content_start != std::string::npos && content_start + 1 <= prev_end) {
								result = html.substr(content_start + 1, prev_end - content_start - 1);
							}
						}
					}
				}
				outputText->SetValue(wxString::FromUTF8(result));
			}
			else if (evt.GetState() == wxWebRequest::State_Failed) {
                outputText->SetValue("HTTP request error");
            }
        });
        req.Start();
    }

    ~MyFrame() override
    {
        if (trayIcon) {
            trayIcon->RemoveIcon();
            delete trayIcon;
        }
    }

private:
    wxDECLARE_EVENT_TABLE();
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


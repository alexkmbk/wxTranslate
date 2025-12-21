#include <wx/wx.h>
#include <wx/webrequest.h>
#include <sstream>
#include <iomanip>

#include <string>
#include <cctype>

#include "translate.h"
#include "settings.h"
#include "lang_panel.h"

bool is_lang_code(const std::string& s) {
    return s.size() == 2 && std::isalpha(s[0]) && std::isalpha(s[1]);
}

static std::string UrlEncode(const std::string& value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;
    for (char c : value) {
        if (isalnum((unsigned char)c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        }
        else {
            escaped << '%' << std::setw(2) << int((unsigned char)c);
        }
    }
    return escaped.str();
}

void translate(wxFrame* frame, wxString text, std::function<void(const std::string& translated, const std::string& sourceLanguage, const std::string& error)> callback) {

    OutputDebugStringA("translate\n");

    if (text.IsEmpty()) {
        callback("", "", "");
        return;
    }

    std::string query{ text.ToUTF8() };
    std::string encoded = UrlEncode(query);

    wxString url = wxString::Format(
        "https://translate.googleapis.com/translate_a/t?client=gtx&sl=%s&tl=%s&dt=t&q=%s",
        g_Settings.currentLangIn, g_Settings.currentLangOut, encoded
    );

    wxWebRequest req = wxWebSession::GetDefault().CreateRequest(frame, url);

    frame->Bind(wxEVT_WEBREQUEST_STATE, [callback](wxWebRequestEvent& evt) {
        if (evt.GetState() == wxWebRequest::State_Completed) {
            wxString response = evt.GetResponse().AsString();
            std::string json = std::string(response.ToUTF8());

            std::vector<std::string> tokens;
            std::string current;

            bool in_string = false;
            bool escape = false;

            for (char c : json) {
                if (!in_string) {
                    if (c == '"') {
                        in_string = true;
                        current.clear();
                    }
                    continue;
                }

                // мы внутри строки
                if (escape) {
                    // обрабатываем экранированные символы
                    switch (c) {
                    case '"':  current.push_back('"');  break;
                    case '\\': current.push_back('\\'); break;
                    case 'n':  current.push_back('\n'); break;
                    case 't':  current.push_back('\t'); break;
                    case 'r':  current.push_back('\r'); break;
                    default:   current.push_back(c);    break;
                    }
                    escape = false;
                    continue;
                }

                if (c == '\\') {
                    escape = true;
                    continue;
                }

                if (c == '"') {
                    // конец строки
                    tokens.push_back(current);
                    in_string = false;
                    continue;
                }

                current.push_back(c);
            }

            if (tokens.size() != 2) {
                callback("", "", "Parse error: invalid response format\n");
                return;
            }

            std::string translated = tokens[0];
            std::string source_lang = tokens[1];

            callback(translated, source_lang, "");
        }
        else if (evt.GetState() == wxWebRequest::State_Failed) {
            callback("", "", "HTTP request error");
        }
    });
    req.Start();
}
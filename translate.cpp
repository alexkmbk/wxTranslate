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

    // Replace ". " with "."
    size_t pos_dot = 0;
    while ((pos_dot = query.find(". ", pos_dot)) != std::string::npos) {
        query.replace(pos_dot, 2, ".");
        pos_dot += 1; // Move past the replaced '.'
    }
    std::string encoded = UrlEncode(query);

    wxString url = wxString::Format(
        "https://translate.googleapis.com/translate_a/single?client=gtx&sl=%s&tl=%s&dt=t&q=%s",
        g_Settings.currentLangIn, g_Settings.currentLangOut, encoded
    );

    wxWebRequest req = wxWebSession::GetDefault().CreateRequest(frame, url);

    frame->Bind(wxEVT_WEBREQUEST_STATE, [callback](wxWebRequestEvent& evt) {
        if (evt.GetState() == wxWebRequest::State_Completed) {
            wxString response = evt.GetResponse().AsString();

            // Parse the penultimate <div> from the end
            std::string json = std::string(response.ToUTF8());

            std::vector<std::string> tokens;
            size_t pos = 0;

            while (true) {
                size_t start = json.find('"', pos);
                if (start == std::string::npos) break;

                size_t end = json.find('"', start + 1);
                if (end == std::string::npos) break;

                tokens.push_back(json.substr(start + 1, end - start - 1));
                pos = end + 1;
            }

            if (tokens.size() < 2) {
                callback("", "", "Parse error: no data\n");
                return;
            }

            // The first two are always translation and original
            std::string translated = tokens[0];
            std::string original = tokens[1];

            // The first ISO language (source)
            std::string source_lang;
            for (const auto& t : tokens) {
                if (is_lang_code(t)) {
                    source_lang = t;
                    break;
                }
            }

            if (tokens.size() < 3) {
                callback("", "", "parsing result error");
                return;
            }

            callback(translated, source_lang, "");
        }
        else if (evt.GetState() == wxWebRequest::State_Failed) {
            callback("", "", "HTTP request error");
        }
    });
    req.Start();
}
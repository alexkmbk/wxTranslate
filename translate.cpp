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
        //textCtrl->SetValue("");
		callback("", "", "");
        return;
	}

    std::string query{ text.ToUTF8() };
    std::string encoded = UrlEncode(query);

 //   wxString sl = langDetector.Detect(query);
 //   wxString tl = "en";

	//if (sl == "unknown"){
 //       sl = "ru";
 //   }

    //if (DetectLang(query) == Lang::English) {
    //    sl = "en";
    //    tl = "ru";
	//}

	// https://translate.googleapis.com/translate_a/single?client=gtx&sl=auto&tl=en&dt=t&dj=1&q=текст

	wxString url = wxString::Format(
		"https://translate.googleapis.com/translate_a/single?client=gtx&sl=%s&tl=%s&dt=t&q=%s",
		g_Settings.currentLangIn, g_Settings.currentLangOut, encoded
	);

	/*wxString url = wxString::Format(
		"https://translate.google.com/m?sl=%s&tl=%s&hl=%s&q=%s",
		sl, tl, tl, encoded
	);*/

    wxWebRequest req = wxWebSession::GetDefault().CreateRequest(frame, url);

    frame->Bind(wxEVT_WEBREQUEST_STATE, [callback](wxWebRequestEvent& evt) {
        if (evt.GetState() == wxWebRequest::State_Completed) {
            wxString response = evt.GetResponse().AsString();

            // Парсим предпоследний <div> с конца
            //std::string html = std::string(response.ToUTF8());
            //std::string result;
            //size_t end = html.rfind("</div>");
            //if (end != std::string::npos) {
            //    // Найти второй с конца </div>
            //    size_t prev_end = html.rfind("</div>", end - 1);
            //    if (prev_end != std::string::npos) {
            //        // Найти начало <div ...> перед prev_end
            //        size_t div_start = html.rfind("<div", prev_end);
            //        if (div_start != std::string::npos) {
            //            size_t content_start = html.find('>', div_start);
            //            if (content_start != std::string::npos && content_start + 1 <= prev_end) {
            //                result = html.substr(content_start + 1, prev_end - content_start - 1);
            //            }
            //        }
            //    }
            //}

            std::string json = std::string(response.ToUTF8());

            std::vector<std::string> tokens;
            size_t pos = 0;

            //while (true) {
            //    size_t start = json.find('"', pos);
            //    if (start == std::string::npos) break;

            //    size_t end = json.find('"', start + 1);
            //    if (end == std::string::npos) break;

            //    tokens.push_back(json.substr(start + 1, end - start - 1));
            //    pos = end + 1;
            //}

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

            // Первые два всегда перевод и оригинал
            std::string translated = tokens[0];
            std::string original = tokens[1];

            // Первый ISO-язык (source)
            std::string source_lang;
            for (const auto& t : tokens) {
                if (is_lang_code(t)) {
                    source_lang = t;
                    break;
                }
            }

            if (tokens.size() < 3) {
                //textCtrl->SetValue(wxString("parsing result error"));
                callback("", "", "parsing result error");
                return;
            }

            //std::string translated = tokens[0];
            //std::string original = tokens[1];
            //std::string source_language = tokens[2];

            //textCtrl->SetValue(wxString::FromUTF8(translated));
            callback(translated, source_lang, "");
        }
        else if (evt.GetState() == wxWebRequest::State_Failed) {
            //textCtrl->SetValue(wxString("HTTP request error"));
            callback("", "", "HTTP request error");
        }
    });
    req.Start();
}
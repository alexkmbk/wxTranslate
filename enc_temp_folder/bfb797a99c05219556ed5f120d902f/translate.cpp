#include <wx/wx.h>
#include <wx/webrequest.h>
#include <sstream>
#include <iomanip>

#include <string>
#include <cctype>

#include "translate.h"
#include "settings.h"

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

void translate(wxFrame* frame, wxTextCtrl* textCtrl, wxString text) {

    OutputDebugStringA("translate\n");

    if (text.IsEmpty()) {
        textCtrl->SetValue("");
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

	// https://translate.googleapis.com/translate_a/single?client=gtx&sl=auto&tl=en&dt=t&q=текст

	wxString url = wxString::Format(
		"https://translate.googleapis.com/translate_a/single?client=gtx&sl=%s&tl=%s&dt=t&q=%s",
		g_Settings.currentLangIn, g_Settings.currentLangOut, encoded
	);

	/*wxString url = wxString::Format(
		"https://translate.google.com/m?sl=%s&tl=%s&hl=%s&q=%s",
		sl, tl, tl, encoded
	);*/

    wxWebRequest req = wxWebSession::GetDefault().CreateRequest(frame, url);

    frame->Bind(wxEVT_WEBREQUEST_STATE, [frame, textCtrl](wxWebRequestEvent& evt) {
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

            while (true) {
                size_t start = json.find('"', pos);
                if (start == std::string::npos) break;

                size_t end = json.find('"', start + 1);
                if (end == std::string::npos) break;

                tokens.push_back(json.substr(start + 1, end - start - 1));
                pos = end + 1;
            }

            if (tokens.size() < 3) {
                textCtrl->SetValue(wxString("parsing result error"));
                return;
            }

            std::string translated = tokens[0];                // "text"
            std::string original = tokens[1];                // "текст"
            std::string source_language = tokens[2];                // "ru"
            //std::string target_language = tokens[tokens.size() - 1];  // последняя "ru"

            textCtrl->SetValue(wxString::FromUTF8(translated));
        }
        else if (evt.GetState() == wxWebRequest::State_Failed) {
            textCtrl->SetValue(wxString("HTTP request error"));
        }
        });
    req.Start();
}
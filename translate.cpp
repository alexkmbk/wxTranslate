#include <wx/wx.h>
#include <wx/webrequest.h>
#include <sstream>
#include <iomanip>

#include <string>
#include <cctype>

#include "lang_detect.h"
#include "translate.h"
#include "settings.h"

SimpleLangDetector langDetector;

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

    wxString sl = langDetector.Detect(query);
    wxString tl = "en";

	if (sl == "unknown"){
        sl = "ru";
    }

    //if (DetectLang(query) == Lang::English) {
    //    sl = "en";
    //    tl = "ru";
    //}

    // https://translate.googleapis.com/translate_a/single?client=gtx&sl=auto&tl=en&dt=t&q=текст

    wxString url = wxString::Format(
        "https://translate.google.com/m?sl=%s&tl=%s&hl=%s&q=%s",
        sl, tl, tl, encoded
    );

    wxWebRequest req = wxWebSession::GetDefault().CreateRequest(frame, url);

    frame->Bind(wxEVT_WEBREQUEST_STATE, [frame, textCtrl](wxWebRequestEvent& evt) {
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
            textCtrl->SetValue(wxString::FromUTF8(result));
        }
        else if (evt.GetState() == wxWebRequest::State_Failed) {
            textCtrl->SetValue(wxString("HTTP request error"));
        }
        });
    req.Start();
}

// Определение языка: русский / английский
Lang DetectLang(const std::string& text) {
    bool hasCyrillic = false;
    bool hasLatin = false;

    // UTF-8 → проверяем байты
    for (size_t i = 0; i < text.size(); ) {
        unsigned char c = text[i];

        if (c < 0x80) {
            // ASCII (латиница)
            if (std::isalpha(c)) {
                hasLatin = true;
            }
            i++;
        }
        else {
            // Много байтов (UTF-8)
            if ((c & 0xE0) == 0xC0 && i + 1 < text.size()) {
                unsigned int ch = ((c & 0x1F) << 6) | (text[i + 1] & 0x3F);
                if (ch >= 0x0410 && ch <= 0x044F) { // А–я
                    hasCyrillic = true;
                }
                i += 2;
            }
            else if ((c & 0xF0) == 0xE0 && i + 2 < text.size()) {
                unsigned int ch = ((c & 0x0F) << 12) |
                    ((text[i + 1] & 0x3F) << 6) |
                    (text[i + 2] & 0x3F);
                if (ch >= 0x0410 && ch <= 0x044F) { // А–я
                    hasCyrillic = true;
                }
                i += 3;
            }
            else {
                i++; // пропускаем непонятные байты
            }
        }
    }

    if (hasCyrillic && !hasLatin) return Lang::Russian;
    if (hasLatin && !hasCyrillic) return Lang::English;
    return Lang::Unknown;
}

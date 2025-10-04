#include <string>
#include <cctype>

#include "lang.h"


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

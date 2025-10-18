#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <cmath>
#include <string_view>

constexpr bool is_latin_lang(std::string_view lang) noexcept {
    return lang == "en" || lang == "de" || lang == "es" ||
        lang == "fr" || lang == "it" || lang == "pt" ||
        lang == "tr" || lang == "vi";
}

struct LangProfile {
    std::string lang;
    std::vector<std::u32string> common;
    std::vector<std::u32string> unique;
    std::unordered_set<char32_t> charset; // Характерные символы

    LangProfile(std::string l,
        std::vector<std::u32string> c,
        std::vector<std::u32string> u,
        std::unordered_set<char32_t> cs = {})
        : lang(std::move(l)), common(std::move(c)), unique(std::move(u)), charset(std::move(cs)) {
    }
};

class SimpleLangDetector {
public:
    SimpleLangDetector()
        : profiles({
            {"en",
                {
                    U"th",U"he",U"in",U"er",U"an",U"re",U"on",U"at",U"en",U"nd",
                    U"ti",U"es",U"or",U"te",U"of",U"ed",U"is",U"it",U"al",U"ar",
                    U"st",U"to",U"nt",U"ha",U"ou",U"se",U"le",U"ve",U"me",U"ne"
                },
                {
                    U"the",U"and",U"ing",U"ion",U"ent",U"tio",U"for",U"tha",U"her",U"ver",
                    U"all",U"with",U"you",U"not",U"was",U"are",U"his",U"but",U"out",U"had"
                },
                {}
            },
            {"fr",
                {
                    U"le",U"de",U"re",U"en",U"on",U"es",U"nt",U"ou",U"an",U"ai",
                    U"er",U"se",U"te",U"la",U"et",U"me",U"ne",U"ce",U"qu",U"co"
                },
                {
                    U"les",U"des",U"ent",U"ion",U"que",U"une",U"pour",U"dans",U"elle",U"vous",
                    U"avec",U"mais",U"tout",U"plus",U"nous",U"bien",U"sans",U"sera",U"étai"
                },
                {U'é', U'è', U'ê', U'à', U'ç', U'ù', U'û', U'ô', U'î', U'ï'}
            },
            {"de",
                {
                    U"en",U"er",U"ch",U"de",U"ei",U"nd",U"te",U"in",U"ge",U"un",
                    U"ie",U"es",U"st",U"be",U"an",U"re",U"au",U"ic",U"di",U"sc"
                },
                {
                    U"der",U"die",U"und",U"den",U"ein",U"nicht",U"das",U"sie",U"wie",U"zum",
                    U"sch",U"ung",U"nde",U"mit",U"dem",U"des",U"aus",U"von",U"hab",U"über",
                    U"chen",U"lich",U"keit",U"frei",U"mensch",U"straße",U"ganz",U"sich",U"einer"
                },
                {U'ä', U'ö', U'ü', U'ß'}
            },
            {"es",
                {
                    U"de",U"en",U"es",U"el",U"la",U"os",U"as",U"ar",U"er",U"se",
                    U"ue",U"ra",U"or",U"nt",U"te",U"re",U"co",U"ci",U"lo",U"an"
                },
                {
                    U"que",U"los",U"las",U"una",U"con",U"por",U"para",U"del",U"est",U"era",
                    U"como",U"más",U"pero",U"sus",U"cuando",U"todo",U"esta",U"muy",U"bien",U"solo"
                },
                {U'ñ', U'á', U'é', U'í', U'ó', U'ú', U'ü', U'¿', U'¡'}
            },
            {"it",
                {
                    U"di",U"re",U"ni",U"ta",U"on",U"ne",U"ti",U"si",U"ri",U"to",
                    U"li",U"no",U"la",U"le",U"co",U"io",U"el",U"an",U"ra",U"lo"
                },
                {
                    U"che",U"per",U"con",U"una",U"non",U"del",U"all",U"ell",U"tto",U"lla",
                    U"gli",U"que",U"ssa",U"ndo",U"ere",U"ano",U"tta",U"zione",U"mento",U"questo"
                },
                {U'à', U'è', U'é', U'ì', U'ò', U'ù'}
            },
            {"pt",
                {
                    U"de",U"es",U"do",U"os",U"ra",U"se",U"nt",U"as",U"re",U"or",
                    U"te",U"co",U"em",U"da",U"no",U"me",U"ta",U"po",U"is",U"al"
                },
                {
                    U"ção",U"que",U"lhe",U"nha",U"ões",U"para",U"como",U"mais",U"esta",U"tamb",
                    U"isso",U"pois",U"qual",U"dela",U"entre",U"quando",U"ainda",U"foram",U"porque",U"tudo"
                },
                {U'ã', U'õ', U'ç', U'á', U'à', U'â', U'é', U'ê', U'í', U'ó', U'ô', U'ú'}
            },
            {"tr",
                {
                    U"ve",U"de",U"da",U"en",U"in",U"ar",U"an",U"er",U"ir",U"il",
                    U"bi",U"si",U"ya",U"ni",U"li",U"ne",U"di",U"ta",U"le",U"ki"
                },
                {
                    U"bir",U"için",U"çok",U"ama",U"gibi",U"şey",U"olarak",U"bunu",U"çünkü",U"kadar",
                    U"yani",U"şimdi",U"burada",U"bütün",U"değil",U"kend",U"diye",U"bile"
                },
                {U'ç', U'ğ', U'ı', U'İ', U'ö', U'ş', U'ü'}
            },
            {"vi",
                {
                    U"ng",U"nh",U"th",U"tr",U"ph",U"ch",U"qu",U"gi",U"gh",U"kh"
                },
                {
                    U"ng",U"nh",U"tr",U"ph",U"đ",U"không",U"những",U"được",U"này",U"nhiều"
                },
                {U'ă', U'â', U'đ', U'ê', U'ô', U'ơ', U'ư', U'á', U'à', U'ả', U'ã', U'ạ'}
            },
            {"ru",
                {
                    U"ст",U"но",U"ен",U"то",U"на",U"ни",U"пр",U"ра",U"ос",U"во",
                    U"ов",U"ро",U"ко",U"не",U"по",U"ва",U"ет",U"ли",U"ка",U"от"
                },
                {
                    U"что",U"это",U"как",U"был",U"если",U"всё",U"для",U"или",U"том",U"они"
                },
                {U'а',U'б',U'в',U'г',U'д',U'е',U'ё',U'ж',U'з',U'и',U'й',U'к',U'л',U'м',
                 U'н',U'о',U'п',U'р',U'с',U'т',U'у',U'ф',U'х',U'ц',U'ч',U'ш',U'щ',U'ъ',
                 U'ы',U'ь',U'э',U'ю',U'я'}
            },
            {"zh",
                {
                    U"的",U"是",U"我",U"有",U"人",U"不",U"这",U"中",U"国",U"们",
                    U"在",U"了",U"和",U"会",U"对",U"个",U"上",U"大",U"也",U"为"
                },
                {},
                {} // CJK диапазон проверяется отдельно
            },
            {"ja",
                {
                    U"の",U"に",U"て",U"は",U"を",U"が",U"し",U"た",U"で",U"す",
                    U"と",U"る",U"も",U"れ",U"だ",U"う",U"ん",U"な",U"い",U"か"
                },
                {},
                {U'あ',U'い',U'う',U'え',U'お',U'か',U'き',U'く',U'け',U'こ'} // Хирагана (примеры)
            },
            {"ar",
                {
                    U"ال",U"من",U"في",U"أن",U"على",U"ما",U"لا",U"كان",U"عن",U"هو",
                    U"إلى",U"هذا",U"قد",U"أو",U"إن",U"بن",U"كل",U"لم",U"ها",U"بي"
                },
                {},
                {U'ا',U'ب',U'ت',U'ث',U'ج',U'ح',U'خ',U'د',U'ذ',U'ر',U'ز',U'س',U'ش',U'ص',
                 U'ض',U'ط',U'ظ',U'ع',U'غ',U'ف',U'ق',U'ك',U'ل',U'م',U'ن',U'ه',U'و',U'ي'}
            },
            {"hi",
                {
                    U"कि",U"कर",U"का",U"रा",U"ता",U"ना",U"है",U"यह",U"में",U"और",
                    U"को",U"से",U"पर",U"हो",U"जा",U"दि",U"सा",U"मा",U"गा",U"था"
                },
                {},
                {U'अ',U'आ',U'इ',U'ई',U'उ',U'ऊ',U'ए',U'ऐ',U'ओ',U'औ',U'क',U'ख',U'ग',U'घ',
                 U'च',U'छ',U'ज',U'झ',U'ट',U'ठ',U'ड',U'ढ',U'ण',U'त',U'थ',U'द',U'ध',U'न',
                 U'प',U'फ',U'ब',U'भ',U'म',U'य',U'र',U'ल',U'व',U'श',U'ष',U'स',U'ह'}
            },
            {"fa",
                {
                    U"از",U"به",U"در",U"را",U"که",U"این",U"با",U"یک",U"است",U"بر",
                    U"ای",U"ها",U"هر",U"تا",U"شد",U"دو",U"خود",U"ان",U"هم",U"شو"
                },
                {},
                {U'آ',U'ا',U'ب',U'پ',U'ت',U'ث',U'ج',U'چ',U'ح',U'خ',U'د',U'ذ',U'ر',U'ز',
                 U'ژ',U'س',U'ش',U'ص',U'ض',U'ط',U'ظ',U'ع',U'غ',U'ف',U'ق',U'ک',U'گ',U'ل',
                 U'م',U'ن',U'و',U'ه',U'ی'}
            },
            {"kz",
                {
                    U"ен",U"ал",U"де",U"та",U"ға",U"ды",U"да",U"ар",U"ер",U"ке",
                    U"па",U"ра",U"ан",U"ба",U"не",U"са",U"ме",U"на",U"ма",U"ла"
                },
                {
                    U"мен",U"сен",U"бұл",U"бар",U"жоқ",U"еді",U"тағы",U"қала",U"адам",U"үй",
                    U"және",U"болу",U"деп",U"сол",U"үшін",U"өте",U"ең",U"қазақ"
                },
                {U'ә',U'ғ',U'қ',U'ң',U'ө',U'ұ',U'ү',U'һ',U'і'}
            }
            })
    {
        // Предварительная обработка: конвертация в unordered_set для быстрого поиска
        for (auto& prof : profiles) {
            prof.common_set.insert(prof.common.begin(), prof.common.end());
            prof.unique_set.insert(prof.unique.begin(), prof.unique.end());
        }
    }

    std::string Detect(const std::string& text) const {
        if (text.empty()) return "unknown";

        // Извлекаем символы и n-граммы
        auto u32text = utf8_to_u32(text);
        normalize(u32text);

        if (u32text.size() < 2) return "unknown";

        auto bigrams = extractNgrams(u32text, 2);
        auto trigrams = extractNgrams(u32text, 3);

        std::unordered_set<std::u32string> bigram_set(bigrams.begin(), bigrams.end());
        std::unordered_set<std::u32string> trigram_set(trigrams.begin(), trigrams.end());
        std::unordered_set<char32_t> char_set(u32text.begin(), u32text.end());

        std::unordered_map<std::string, double> scores;

        for (const auto& prof : profiles) {
            double score = 0.0;

            // 1. Проверка специфичных символов (сильный индикатор)
            if (!prof.charset.empty()) {
                int charset_matches = 0;
                for (char32_t ch : char_set) {
                    if (prof.charset.contains(ch)) {
                        ++charset_matches;
                    }
                }
                double charset_ratio = static_cast<double>(charset_matches) /
                    std::max(1.0, static_cast<double>(prof.charset.size()));
                score += charset_ratio * 3.0; // Высокий вес для уникальных символов
            }

            // 2. Проверка биграмм
            int common_matches = 0;
            for (const auto& ng : prof.common) {
                if (bigram_set.contains(ng)) ++common_matches;
            }

            int unique_matches = 0;
            for (const auto& ng : prof.unique) {
                if (bigram_set.contains(ng) || trigram_set.contains(ng)) {
                    ++unique_matches;
                }
            }

            // Нормализация
            double common_score = prof.common.empty() ? 0.0 :
                static_cast<double>(common_matches) / prof.common.size();

            double unique_score = prof.unique.empty() ? 0.0 :
                static_cast<double>(unique_matches) / prof.unique.size();

            // Взвешенная сумма
            score += common_score * 1.0 + unique_score * 2.5;

            // 3. Штраф за отсутствие уникальных совпадений при наличии профиля
            if (!prof.unique.empty() && unique_matches == 0) {
                score *= 0.5;
            }

            scores[prof.lang] = score;
        }

        // Находим лучший результат
        auto best = std::max_element(scores.begin(), scores.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });

        if (best == scores.end() || best->second < 0.15) {
            return "unknown";
        }

        // Собираем близкие результаты (разница менее 20%)
        std::vector<std::pair<std::string, double>> close_results;
        for (const auto& [lang, score] : scores) {
            if (score >= best->second * 0.8) {
                close_results.push_back({ lang, score });
            }
        }

        // Если есть несколько близких результатов среди латиницы
        if (close_results.size() > 1) {
            // Определяем языки на латинице без уникальных символов
            //std::unordered_set<std::string> latin_langs = { "en", "de", "es", "fr", "it", "pt", "tr", "vi" };

            // Проверяем, есть ли специфичные символы в тексте
            bool has_special_chars = false;
            for (char32_t ch : char_set) {
                // Проверяем наличие диакритических знаков и специальных символов
                if ((ch > 127 && ch < 0x0370) || // Латинские расширения
                    ch == U'ñ' || ch == U'ç' || ch == U'ß' || ch == U'ø' || ch == U'å' ||
                    ch == U'ğ' || ch == U'ş' || ch == U'ı' || ch == U'đ' || ch == U'ă' ||
                    ch == U'â' || ch == U'ê' || ch == U'ô' || ch == U'ơ' || ch == U'ư') {
                    has_special_chars = true;
                    break;
                }
            }

            // Проверяем, все ли близкие результаты - латинские языки
            bool all_latin = true;
            bool has_english = false;
            for (const auto& [lang, score] : close_results) {
                if (!is_latin_lang(lang)) {
                    all_latin = false;
                    break;
                }
                if (lang == "en") {
                    has_english = true;
                }
            }

            // Если все близкие результаты - латинские языки, нет спецсимволов, 
            // и среди них есть английский - выбираем английский
            if (all_latin && !has_special_chars && has_english) {
                return "en";
            }
        }

        // Проверка на значительное превосходство
        auto second_best = scores.end();
        for (auto it = scores.begin(); it != scores.end(); ++it) {
            if (it != best && (second_best == scores.end() || it->second > second_best->second)) {
                second_best = it;
            }
        }

        // Если разница слишком мала, возвращаем "unknown"
        if (second_best != scores.end() && best->second / (second_best->second + 0.01) < 1.3) {
            return "unknown";
        }

        return best->first;
    }

private:
    struct ExtendedLangProfile : LangProfile {
        std::unordered_set<std::u32string> common_set;
        std::unordered_set<std::u32string> unique_set;

        using LangProfile::LangProfile;
    };

    std::vector<ExtendedLangProfile> profiles;

    static std::u32string utf8_to_u32(const std::string& s) {
        std::u32string out;
        out.reserve(s.size()); // Оптимизация

        for (size_t i = 0; i < s.size();) {
            unsigned char c = s[i];
            char32_t code = 0;
            size_t extra = 0;

            if (c < 0x80) {
                code = c;
                extra = 0;
            }
            else if ((c >> 5) == 0x6) {
                code = c & 0x1F;
                extra = 1;
            }
            else if ((c >> 4) == 0xE) {
                code = c & 0x0F;
                extra = 2;
            }
            else if ((c >> 3) == 0x1E) {
                code = c & 0x07;
                extra = 3;
            }
            else {
                i++;
                continue;
            }

            if (i + extra >= s.size()) break;

            for (size_t j = 1; j <= extra; ++j) {
                code = (code << 6) | (s[i + j] & 0x3F);
            }

            out.push_back(code);
            i += extra + 1;
        }
        return out;
    }

    static void normalize(std::u32string& str) {
        for (auto& c : str) {
            if (c >= U'A' && c <= U'Z') {
                c = static_cast<char32_t>(c + (U'a' - U'A'));
            }
            // Нормализация кириллических заглавных букв
            else if (c >= U'А' && c <= U'Я') {
                c = static_cast<char32_t>(c + (U'а' - U'А'));
            }
        }
    }

    static std::vector<std::u32string> extractNgrams(const std::u32string& u32, size_t n) {
        std::vector<std::u32string> result;
        if (u32.size() < n) return result;

        result.reserve(u32.size() - n + 1);
        for (size_t i = 0; i + n <= u32.size(); ++i) {
            result.emplace_back(u32.substr(i, n));
        }
        return result;
    }
};

// Пример использования
int main() {
    SimpleLangDetector detector;

    std::vector<std::string> tests = {
        "Hello, how are you doing today?",
        "Bonjour, comment allez-vous?",
        "Привет, как дела?",
        "Hola, ¿cómo estás?",
        "Guten Tag, wie geht es Ihnen?",
        "Сәлеметсіз бе, қалайсыз?",
        "مرحبا كيف حالك؟",
        "こんにちは、お元気ですか？"
    };

    for (const auto& text : tests) {
        std::cout << "Text: " << text << "\nDetected: " << detector.Detect(text) << "\n\n";
    }

    return 0;
}
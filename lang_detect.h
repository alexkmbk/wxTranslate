#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>

struct LangProfile {
	std::string lang;
	std::vector<std::u32string> common;
	std::vector<std::u32string> unique;

	LangProfile(const std::string& l,
		const std::vector<std::u32string>& c,
		const std::vector<std::u32string>& u)
		: lang(l), common(c), unique(u) {
	}
};

class SimpleLangDetector {
public:
	SimpleLangDetector()
		: profiles({
			{"en",
				toU32({
					"th","he","in","er","an","re","on","at","en","nd",
					"ti","es","or","te","of","ed","is","it","al","ar",
					"st","to","nt","ha","ou","se","le","ve","me","ne"
				}),
				toU32({
					"the","and","ing","ion","ent","tio","for","tha","her","ver",
					"all","with","you","not","was","are","his","but","out","had"
				})
			},
			{"fr",
				toU32({
					"le","de","re","en","on","es","nt","ou","an","ai",
					"er","se","te","la","et","me","ne","ce","qu","co"
				}),
				toU32({
					"les","des","ent","ion","que","une","pour","dans","elle","vous",
					"avec","mais","tout","plus","nous","bien","sans","sera","étai"
				})
			},
			{"de",
			toU32({
				// Additions that include unique German characteristics
				"ä","ö","ü","ß","sch","ei","ig","ich","au","ge"
			}),
			toU32({
				"der","die","und","den","ein","nicht","das","sie","wie","zum",
				"sch","ung","nde","mit","dem","des","aus","von","hab","über",
				"chen","lich","keit","frei","mensch","über","straße","ganz","sich","einer"
			})
			},
			{"es",
				toU32({
					"de","en","es","el","la","os","as","ar","er","se",
					"ue","ra","or","nt","te","re","co","ci","lo","an"
				}),
				toU32({
					"que","los","las","una","con","por","para","del","est","era",
					"como","más","pero","sus","cuando","todo","esta","muy","bien","solo"
				})
			},
			{"it",
				toU32({
					"di","re","ni","ta","on","ne","ti","si","ri","to",
					"li","no","la","le","co","io","el","an","ra","lo"
				}),
				toU32({
					"che","per","con","una","non","del","all","ell","tto","lla",
					"gli","que","ssa","ndo","ere","ano","tta","zione","mento","questo"
				})
			},
			{"pt",
				toU32({
					"de","es","do","os","ra","se","nt","as","re","or",
					"te","co","em","da","no","me","ta","po","is","al"
				}),
				toU32({
					"ção","que","lhe","nha","ões","para","como","mais","esta","tamb",
					"isso","pois","qual","dela","entre","quando","ainda","foram","porque","tudo"
				})
			},
			{"tr",
				toU32({
					"ve","de","da","en","in","ar","an","er","ir","il",
					"bi","si","ya","ni","li","ne","di","ta","le","ki"
				}),
				toU32({
					"bir","için","çok","ama","gibi","şey","olarak","bunu","çünkü","kadar",
					"olarak","yani","şimdi","burada","bütün","şimdi","değil","kend","diye","bile"
				})
			},
			{"vi",
				toU32({
					"ng","nh","th","tr","ph","ch","ă","â","đ","ư"
				}),
				toU32({
					"ng","nh","tr","ph","đ"
				})
			},
			{"ru",
				toU32({
					"ст","но","ен","то","на","ни","пр","ра","ос","во"
				}),
				{}
			},
			{"zh",
				toU32({
					"的","是","我","有","人","不","这","中","国","们"
				}),
				{}
			},
			{"ja",
				toU32({
					"の","に","て","は","を","が","し","た","で","す"
				}),
				{}
			},
			{"ar",
				toU32({
					"ال","من","في","أن","على","ما","لا","كان","عن","هو"
				}),
				{}
			},
			{"hi",
				toU32({
					"कि","कर","का","रा","ता","ना","है","यह","में","और"
				}),
				{}
			},
			{"fa",
				toU32({
					"من","تو","است","برای","این","که","در","از","با","هم"
				}),
				{}
			},
			{"kz",
				toU32({
					"мен","сен","бұл","бар","жоқ","еді","тағы","қала","адам","үй"
				}),
				{}
			}
			})
	{}

	std::string Detect(const std::string& text) const {
		if (text.empty()) return "unknown";

		auto ngrams = extractNgrams(text, 2);
		std::unordered_set<std::u32string> set(ngrams.begin(), ngrams.end());
		std::unordered_map<std::string, double> scores;

		for (const auto& prof : profiles) {
			int common = 0, unique = 0;

			// Подсчет совпадений в общих n-граммах
			for (const auto& ng : prof.common)
				if (set.contains(ng)) ++common;

			// Подсчет совпадений в уникальных n-граммах
			for (const auto& ng : prof.unique)
				if (set.contains(ng)) ++unique;

			// Нормализация по минимальному размеру (чтобы большие профили не страдали)
			double baseNorm = static_cast<double>(std::min(prof.common.size(), ngrams.size())) + 1;
			double bonusNorm = static_cast<double>(std::min(prof.unique.size(), ngrams.size())) + 1;

			double baseScore = static_cast<double>(common) / baseNorm;
			double bonus = static_cast<double>(unique) / bonusNorm;

			// Увеличенный вес уникальных биграмм
			scores[prof.lang] = baseScore + bonus * 1.5;
		}

		// Поиск лучшего результата
		auto best = std::max_element(scores.begin(), scores.end(),
			[](const auto& a, const auto& b) { return a.second < b.second; });

		if (best == scores.end() || best->second < 0.1)
			return "unknown";

		return best->first;
	}


private:
	std::vector<LangProfile> profiles;

	static std::u32string utf8_to_u32(const std::string& s) {
		std::u32string out;
		for (size_t i = 0; i < s.size();) {
			unsigned char c = s[i];
			char32_t code = 0;
			size_t extra = 0;
			if (c < 0x80) { code = c; extra = 0; }
			else if ((c >> 5) == 0x6) { code = c & 0x1F; extra = 1; }
			else if ((c >> 4) == 0xE) { code = c & 0x0F; extra = 2; }
			else if ((c >> 3) == 0x1E) { code = c & 0x07; extra = 3; }
			else { i++; continue; }
			if (i + extra >= s.size()) break;
			for (size_t j = 1; j <= extra; ++j)
				code = (code << 6) | (s[i + j] & 0x3F);
			out.push_back(code);
			i += extra + 1;
		}
		return out;
	}

	static void normalize(std::u32string& str) {
		std::transform(str.begin(), str.end(), str.begin(), [](char32_t c) -> char32_t {
			if (c >= U'A' && c <= U'Z')
				return static_cast<char32_t>(c + (U'a' - U'A'));
			return c;
			});
	}

	static std::vector<std::u32string> extractNgrams(const std::string& text, size_t n) {
		std::u32string u32 = utf8_to_u32(text);
		normalize(u32);
		std::vector<std::u32string> result;
		if (u32.size() < n) return result;
		for (size_t i = 0; i + n <= u32.size(); ++i)
			result.emplace_back(u32.substr(i, n));
		return result;
	}

	static std::vector<std::u32string> toU32(const std::vector<std::string>& src) {
		std::vector<std::u32string> out;
		out.reserve(src.size());
		for (const auto& s : src)
			out.push_back(utf8_to_u32(s));
		return out;
	}
};

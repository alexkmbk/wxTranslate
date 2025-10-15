#include "utils.h"

#include <string>
#include <sstream>
#include <vector>
std::vector<std::wstring> split(const std::wstring &s, wchar_t delim) {
	std::wstringstream ss(s);
	std::wstring item;
	std::vector<std::wstring> elems;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}
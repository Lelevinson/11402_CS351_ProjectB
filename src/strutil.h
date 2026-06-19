#pragma once

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <string>

namespace csvdb {

// Small string helpers shared across the query modules (header-only).

inline std::string trim(const std::string& s) {
	std::size_t begin = 0;
	std::size_t end = s.size();
	while (begin < end && std::isspace(static_cast<unsigned char>(s[begin]))) {
		++begin;
	}
	while (end > begin && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
		--end;
	}
	return s.substr(begin, end - begin);
}

inline std::string toUpper(const std::string& s) {
	std::string out = s;
	std::transform(out.begin(), out.end(), out.begin(),
	               [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
	return out;
}

// True if index i is in range and points at a whitespace character. Safe for
// i == (some_index - 1) underflow: a wrapped-around value is simply out of range.
inline bool isSpaceAt(const std::string& s, std::size_t i) {
	return i < s.size() && std::isspace(static_cast<unsigned char>(s[i]));
}

}  // namespace csvdb

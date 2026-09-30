#ifndef CHARSET_HPP
#define CHARSET_HPP

#include <string>

// Self-contained GBK <-> UTF-8 conversion (no iconv / boost::locale,
// which are unavailable on Android).
std::string GBKToUtf8(const char* pBuff, size_t Size);
std::string GBKToUtf8(const std::string& String);
std::string GBKToUtf8(const char* pText);
std::string GBKFromUtf8(const std::string& String);

#endif

#include "npafile.hpp"
#include "charset.hpp"

NpaFile::NpaFile(const string& Name) : Name(Name)
{
}

NpaFile::~NpaFile()
{
}

void NpaFile::SetLocale(const char* LocaleStr)
{
    // Charset conversion is built-in (GBK only); kept for API compat.
    (void)LocaleStr;
}

char* NpaFile::Decrypt(char* pBuff, uint32_t Size, uint32_t Offset)
{
    static const uint8_t Key[8] = { 0xBD, 0xAA, 0xBC, 0xB4, 0xAB, 0xB6, 0xBC, 0xB4 };
    for (uint32_t i = 0; i < Size; ++i)
        pBuff[i] ^= Key[(i + Offset) % 8];
    return pBuff;
}

char* NpaFile::Encrypt(char* pBuff, uint32_t Size)
{
    return Decrypt(pBuff, Size);
}

string NpaFile::ToUtf8(const char* pBuff, uint32_t Size)
{
    return GBKToUtf8(pBuff, Size);
}

string NpaFile::ToUtf8(const string& String)
{
    return GBKToUtf8(String);
}

string NpaFile::ToUtf8(const char* pText)
{
    return GBKToUtf8(pText);
}

string NpaFile::FromUtf8(const string& String)
{
    return GBKFromUtf8(String);
}

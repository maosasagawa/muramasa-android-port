#include "charset.hpp"
#include <cstring>
#include <cstdint>
#include <unordered_map>

#include "gbktable.inc"

namespace
{
void AppendUtf8(std::string& Out, uint32_t cp)
{
    if (cp < 0x80)
        Out += (char)cp;
    else if (cp < 0x800)
    {
        Out += (char)(0xC0 | (cp >> 6));
        Out += (char)(0x80 | (cp & 0x3F));
    }
    else if (cp < 0x10000)
    {
        Out += (char)(0xE0 | (cp >> 12));
        Out += (char)(0x80 | ((cp >> 6) & 0x3F));
        Out += (char)(0x80 | (cp & 0x3F));
    }
    else
    {
        Out += (char)(0xF0 | (cp >> 18));
        Out += (char)(0x80 | ((cp >> 12) & 0x3F));
        Out += (char)(0x80 | ((cp >> 6) & 0x3F));
        Out += (char)(0x80 | (cp & 0x3F));
    }
}

const std::unordered_map<uint32_t, uint16_t>& U2G()
{
    static std::unordered_map<uint32_t, uint16_t>* Map = []()
    {
        auto* m = new std::unordered_map<uint32_t, uint16_t>();
        for (int lead = 0; lead < 126; ++lead)
            for (int trail = 0; trail < 256; ++trail)
                if (uint16_t cp = GBK2U[lead][trail])
                    (*m)[cp] = ((lead + 0x81) << 8) | trail;
        return m;
    }();
    return *Map;
}
}

std::string GBKToUtf8(const char* pBuff, size_t Size)
{
    std::string Out;
    Out.reserve(Size);
    for (size_t i = 0; i < Size; ++i)
    {
        uint8_t c = (uint8_t)pBuff[i];
        if (c < 0x80)
        {
            Out += (char)c;
            continue;
        }
        if (c >= 0x81 && i + 1 < Size)
        {
            uint16_t cp = GBK2U[c - 0x81][(uint8_t)pBuff[i + 1]];
            if (cp)
            {
                AppendUtf8(Out, cp);
                ++i;
                continue;
            }
        }
        Out += '?';
    }
    return Out;
}

std::string GBKToUtf8(const std::string& String)
{
    return GBKToUtf8(String.c_str(), String.size());
}

std::string GBKToUtf8(const char* pText)
{
    return GBKToUtf8(pText, strlen(pText));
}

std::string GBKFromUtf8(const std::string& String)
{
    const auto& Map = U2G();
    std::string Out;
    Out.reserve(String.size());
    const unsigned char* s = (const unsigned char*)String.c_str();
    size_t n = String.size(), i = 0;
    while (i < n)
    {
        uint32_t cp;
        size_t adv;
        if (s[i] < 0x80) { cp = s[i]; adv = 1; }
        else if ((s[i] & 0xE0) == 0xC0 && i + 1 < n)
        {
            cp = ((s[i] & 0x1F) << 6) | (s[i + 1] & 0x3F); adv = 2;
        }
        else if ((s[i] & 0xF0) == 0xE0 && i + 2 < n)
        {
            cp = ((s[i] & 0x0F) << 12) | ((s[i + 1] & 0x3F) << 6) | (s[i + 2] & 0x3F); adv = 3;
        }
        else if ((s[i] & 0xF8) == 0xF0 && i + 3 < n)
        {
            cp = ((s[i] & 0x07) << 18) | ((s[i + 1] & 0x3F) << 12) |
                 ((s[i + 2] & 0x3F) << 6) | (s[i + 3] & 0x3F); adv = 4;
        }
        else { cp = 0xFFFD; adv = 1; }
        i += adv;

        if (cp < 0x80)
            Out += (char)cp;
        else
        {
            auto it = Map.find(cp);
            if (it != Map.end())
            {
                Out += (char)(it->second >> 8);
                Out += (char)(it->second & 0xFF);
            }
            else
                Out += '?';
        }
    }
    return Out;
}

#pragma once
// Minimal boost::format replacement supporting %s, %d, %x and width/zero-pad
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>
namespace boost {
class format
{
public:
    format(const char* f) : fmt(f) { }
    format(const std::string& f) : fmt(f) { }
    template <class T>
    format& operator%(const T& v)
    {
        std::ostringstream o;
        o << v;
        args.push_back(o.str());
        return *this;
    }
    std::string str() const
    {
        std::string out;
        size_t ai = 0;
        for (size_t i = 0; i < fmt.size(); ++i)
        {
            if (fmt[i] != '%')
            {
                out += fmt[i];
                continue;
            }
            if (i + 1 < fmt.size() && fmt[i + 1] == '%')
            {
                out += '%';
                ++i;
                continue;
            }
            size_t j = i + 1;
            while (j < fmt.size() && (fmt[j] == '0' || fmt[j] == '-' || fmt[j] == '+' ||
                   fmt[j] == ' ' || fmt[j] == '#' || (fmt[j] >= '0' && fmt[j] <= '9') || fmt[j] == '.'))
                ++j;
            char conv = j < fmt.size() ? fmt[j] : 's';
            std::string spec = fmt.substr(i, j - i + 1);
            std::string arg = ai < args.size() ? args[ai++] : "";
            if (conv == 'd' || conv == 'i' || conv == 'x' || conv == 'X' ||
                conv == 'o' || conv == 'u' || conv == 'c')
            {
                long v = std::strtol(arg.c_str(), nullptr, 0);
                if (conv == 'c')
                {
                    char b[8];
                    snprintf(b, sizeof(b), spec.c_str(), (int)v);
                    out += b;
                }
                else if (conv == 'u')
                {
                    char b[32];
                    snprintf(b, sizeof(b), spec.c_str(), (unsigned)v);
                    out += b;
                }
                else
                {
                    char b[64];
                    snprintf(b, sizeof(b), spec.c_str(), (conv == 'x' || conv == 'X' || conv == 'o') ? (long)(unsigned)v : v);
                    out += b;
                }
            }
            else if (conv == 'f' || conv == 'e' || conv == 'g')
            {
                char b[64];
                snprintf(b, sizeof(b), spec.c_str(), std::strtod(arg.c_str(), nullptr));
                out += b;
            }
            else
            {
                char* b = nullptr;
                int n = snprintf(nullptr, 0, spec.c_str(), arg.c_str());
                if (n > 0)
                {
                    b = new char[n + 1];
                    snprintf(b, n + 1, spec.c_str(), arg.c_str());
                    out += b;
                    delete[] b;
                }
                else
                    out += arg;
            }
            i = j;
        }
        return out;
    }
    operator std::string() const { return str(); }
private:
    std::string fmt;
    std::vector<std::string> args;
};
}

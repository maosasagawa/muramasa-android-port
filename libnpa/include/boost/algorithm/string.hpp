#pragma once
// Minimal boost string algorithms used by libnpengine
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
namespace boost {
namespace algorithm {
inline std::string to_lower_copy(const std::string& s)
{
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(), [](unsigned char c){ return std::tolower(c); });
    return r;
}
}
struct is_any_of_pred
{
    std::string set;
    bool operator()(char c) const { return set.find(c) != std::string::npos; }
};
inline is_any_of_pred is_any_of(const char* s) { return {s}; }
template <class Pred>
inline void split(std::vector<std::string>& out, const std::string& in, Pred pred)
{
    std::string cur;
    for (char c : in)
    {
        if (pred(c))
        {
            if (!cur.empty()) { out.push_back(cur); cur.clear(); }
        }
        else
            cur += c;
    }
    if (!cur.empty())
        out.push_back(cur);
}
}

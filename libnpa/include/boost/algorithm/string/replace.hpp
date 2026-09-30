#pragma once
// Minimal boost::replace_all replacement
#include <string>
namespace boost {
inline void replace_all(std::string& s, const std::string& from, const std::string& to)
{
    if (from.empty())
        return;
    for (std::string::size_type p = 0; (p = s.find(from, p)) != std::string::npos; p += to.size())
        s.replace(p, from.size(), to);
}
}

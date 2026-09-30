#pragma once
// Minimal boost::bimap replacement (header-only, std::map based)
#include <map>
#include <utility>
#include <initializer_list>
namespace boost {
template <class L, class R>
class bimap
{
public:
    typedef std::pair<L, R> value_type;
    std::map<L, R> left;
    std::map<R, L> right;
    bimap() { }
    template <class It> bimap(It a, It b)
    {
        for (; a != b; ++a)
        {
            left[a->first] = a->second;
            right[a->second] = a->first;
        }
    }
    bimap(std::initializer_list<value_type> il) : bimap(il.begin(), il.end()) { }
};
}

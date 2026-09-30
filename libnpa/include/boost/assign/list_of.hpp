#pragma once
// Minimal boost::assign::list_of replacement
#include <vector>
namespace boost { namespace assign {
template <class T>
struct list_of_builder
{
    std::vector<T> v;
    template <class... A>
    list_of_builder& operator()(const A&... a)
    {
        v.emplace_back(a...);
        return *this;
    }
    template <class U>
    operator U() { return U(v.begin(), v.end()); }
};
template <class T, class... A>
list_of_builder<T> list_of(const A&... a)
{
    list_of_builder<T> b;
    b(a...);
    return b;
}
}}

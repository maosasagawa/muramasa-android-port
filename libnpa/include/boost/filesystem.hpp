#pragma once
// Minimal boost::filesystem replacement over std::filesystem
#include <filesystem>
namespace boost { namespace filesystem {
using std::filesystem::path;
inline bool exists(const path& p) { return std::filesystem::exists(p); }
inline bool create_directory(const path& p) { return std::filesystem::create_directory(p); }
inline bool remove(const path& p) { return std::filesystem::remove(p); }
inline std::uintmax_t remove_all(const path& p) { return std::filesystem::remove_all(p); }
}}

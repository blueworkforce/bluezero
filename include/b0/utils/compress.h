#ifndef B0__UTILS__COMPRESS_H__INCLUDED
#define B0__UTILS__COMPRESS_H__INCLUDED

#include <string>

namespace b0
{

namespace utils
{

std::string compress(const std::string &str, int level = -1);
std::string decompress(const std::string &str, size_t size = 0);

} // namespace utils

} // namespace b0

#endif // B0__UTILS__COMPRESS_H__INCLUDED

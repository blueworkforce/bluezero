#ifndef B0__UTILS__COMPRESS_H__INCLUDED
#define B0__UTILS__COMPRESS_H__INCLUDED

#include <string>

std::string compress(const std::string &str, int level = -1);
std::string decompress(const std::string &str);

#endif // B0__UTILS__COMPRESS_H__INCLUDED

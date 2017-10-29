#ifndef COMPRESS_HPP_INCLIUDED
#define COMPRESS_HPP_INCLIUDED

#include <string>

std::string compress(const std::string &str, int level = -1);
std::string decompress(const std::string &str);

#endif // COMPRESS_HPP_INCLIUDED

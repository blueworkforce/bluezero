#ifndef B0__UTILS__ENV_H__INCLUDED
#define B0__UTILS__ENV_H__INCLUDED

#include <b0/b0.h>

#include <string>

namespace b0
{

namespace env
{

B0_EXPORT std::string get(const std::string &var, const std::string &def = "");

B0_EXPORT bool getBool(const std::string &var, bool def = false);

B0_EXPORT int getInt(const std::string &var, int def = 0);

} // namespace env

} // namespace b0

#endif // B0__UTILS__ENV_H__INCLUDED

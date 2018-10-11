#ifndef B0__LOGGER__LEVEL_H__INCLUDED
#define B0__LOGGER__LEVEL_H__INCLUDED

#include <string>

namespace b0
{

namespace logger
{

/*!
 * \brief The level of logging
 */
enum class Level : int
{
    //! The most verbose level
    trace = 0,
    //! Less verbose than trace, more verbose than info
    debug = 10,
    //! The default level, should not cause too much spam on the console
    info = 20,
    //! Warning level
    warn = 30,
    //! Error level
    error = 40,
    //! Fatal error level, after which the node would usually terminate
    fatal = 50
};

/*!
 * \brief The information associated with each log level
 */
struct LevelInfo
{
    std::string str;
    Level level;
    int value;
    int attr;
    int fg;
    int bg;

    //! \brief Ansi escape to apply console style
    std::string ansiEscape() const;

    //! \brief Ansi escape to reset console style
    std::string ansiReset() const;
};

/*!
 * \brief Get log level info indexed by log level string
 */
const LevelInfo & levelInfo(const std::string &str);

/*!
 * \brief Get log level info indexed by log level
 */
const LevelInfo & levelInfo(const Level level);

} // namespace logger

} // namespace b0

#endif // B0__LOGGER__LEVEL_H__INCLUDED

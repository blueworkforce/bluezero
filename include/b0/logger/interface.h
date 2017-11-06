#ifndef B0__LOGGER__INTERFACE_H__INCLUDED
#define B0__LOGGER__INTERFACE_H__INCLUDED

#include <string>

#include <boost/format.hpp>

namespace b0
{

namespace logger
{

/*!
 * \brief Base class to add logging functionalities to nodes
 */
class LogInterface
{
public:
    enum LogLevel { TRACE, DEBUG, INFO, WARN, ERROR, FATAL };

    /*!
     * \brief Log a message to the remote logger, with the default level (INFO)
     */
    virtual void log(std::string message);

    /*!
     * \brief Log a message to the remote logger, at default level, using a format string
     */
    template<typename... Arguments>
    void log(std::string const &fmt, Arguments&&... args)
    {
        try
        {
            boost::format format(fmt);
            log_helper(INFO, format, std::forward<Arguments>(args)...);
        }
        catch(boost::io::too_many_args &ex)
        {
            std::string s = fmt;
            s += " (error during formatting)";
            log(s);
        }
    }

    /*!
     * \brief Log a message to the remote logger, with a specified level
     */
    virtual void log(LogLevel level, std::string message) = 0;

    /*!
     * \brief Log a message using a format string
     */
    template<typename... Arguments>
    void log(LogLevel level, std::string const &fmt, Arguments&&... args)
    {
        try
        {
            boost::format format(fmt);
            log_helper(level, format, std::forward<Arguments>(args)...);
        }
        catch(boost::io::too_many_args &ex)
        {
            std::string s = fmt;
            s += " (error during formatting)";
            log(s);
        }
    }

protected:
    //! \cond HIDDEN_SYMBOLS

    virtual void log_helper(LogLevel level, boost::format &format);

    template<class T, class... Args>
    void log_helper(LogLevel level, boost::format &format, T &&t, Args&&... args)
    {
        return log_helper(level, format % std::forward<T>(t), std::forward<Args>(args)...);
    }

    //! \endcond
};

} // namespace logger

} // namespace b0

#endif // B0__LOGGER__INTERFACE_H__INCLUDED

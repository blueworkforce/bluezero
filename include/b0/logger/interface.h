#ifndef B0__LOGGER__INTERFACE_H__INCLUDED
#define B0__LOGGER__INTERFACE_H__INCLUDED

#include <string>

#include <boost/format.hpp>

#include <b0/b0.h>

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
    /*!
     * The level of logging
     */
    enum LogLevel
    {
        //! The most verbose level
        trace = 0,
        //! Less verbose than TRACE
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
     * \brief Log a message to the remote logger, with a specified level
     */
    virtual void log(LogLevel level, const std::string &message) const = 0;

    /*!
     * \brief Log a message using a format string
     */
    template<typename... Arguments>
    void log(LogLevel level, std::string const &fmt, Arguments&&... args) const
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
            log(level, s);
        }
    }

protected:
    //! \cond HIDDEN_SYMBOLS

	virtual void log_helper(LogLevel level, boost::format &format) const;

    template<class T, class... Args>
    void log_helper(LogLevel level, boost::format &format, T &&t, Args&&... args) const
    {
        return log_helper(level, format % std::forward<T>(t), std::forward<Args>(args)...);
    }

    //! \endcond
};

} // namespace logger

} // namespace b0

#endif // B0__LOGGER__INTERFACE_H__INCLUDED

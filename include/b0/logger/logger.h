#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED

#include <b0/utils/protobufhelpers.hpp>

#include <string>
#include <sstream>
#include <boost/thread.hpp>
#include <boost/format.hpp>

namespace b0
{

class Node;

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

/*!
 * \brief A logger which prints messages to local console.
 */
class LocalLogger : public LogInterface
{
public:
    using LogInterface::log;

    //! Constructor
    LocalLogger(b0::Node *node);

    //! Destructor
    virtual ~LocalLogger();

    virtual void log(LogLevel level, std::string message) override;

    struct LevelInfo
    {
        std::string levelStr;
        int attr;
        int fg;
        int bg;

        std::string ansiEscape();

        std::string ansiReset();
    };

    /*!
     * \brief Return meta-information for the specified level
     */
    virtual LevelInfo levelInfo(LogLevel level);

protected:
    //! The node
    b0::Node &node_;
};

/*!
 * \brief A subclass of LocalLogger which also sends log messages remotely, via a ZeroMQ PUB socket
 */
class Logger : public LocalLogger
{
public:
    using LocalLogger::log;

    //! Construct a Logger for the given named object
    Logger(b0::Node *node);

    virtual ~Logger();

    //! Connect the underlying ZeroMQ PUB socket to the given address
    void connect(std::string addr);

    void log(LogLevel level, std::string message) override;

protected:
    //! The ZeroMQ PUB socket where LogEntry es will be published
    zmq::socket_t pub_socket_;
};

} // namespace logger

} // namespace b0

#endif // LOGGER_H_INCLUDED

#ifndef B0__LOGGER__LOGGER_H__INCLUDED
#define B0__LOGGER__LOGGER_H__INCLUDED

#include <b0/logger/interface.h>

#include <string>
#include <sstream>
#include <boost/thread.hpp>
#include <boost/format.hpp>

namespace b0
{

class Node;

namespace logger_msgs
{

class LogEntry;

} // namespace logger_msgs

namespace logger
{

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

    /*!
     * Log a message to the local console logger (i.e. using std::cout)
     */
    virtual void log(LogLevel level, std::string message) const override;

private:
    struct LevelInfo
    {
        std::string levelStr;
        int attr;
        int fg;
        int bg;

        std::string ansiEscape() const;

        std::string ansiReset() const;
    };

    /*!
     * \brief Return meta-information for the specified level
     */
    virtual LevelInfo levelInfo(LogLevel level) const;

    friend class Console;

protected:
    //! The node
    b0::Node &node_;
};

//! \cond HIDDEN_SYMBOLS

struct LoggerPrivate;

//! \endcond

/*!
 * \brief A subclass of LocalLogger which also sends log messages remotely, via a ZeroMQ PUB socket
 */
class Logger : public LocalLogger
{
public:
    using LocalLogger::log;

    /*!
     * Construct a Logger for the given named object
     */
    Logger(b0::Node *node);

    /*!
     * Logger destructor
     */
    virtual ~Logger();

    /*!
     * Connect the underlying ZeroMQ PUB socket to the given address
     */
    void connect(std::string addr);

    void log(LogLevel level, std::string message) const override;

protected:
    /*!
     * Log a message to the remote logger (i.e. using the log publisher)
     */
    virtual void remoteLog(LogLevel level, std::string message) const;

private:
    mutable std::unique_ptr<LoggerPrivate> private_;
};

} // namespace logger

} // namespace b0

#endif // B0__LOGGER__LOGGER_H__INCLUDED

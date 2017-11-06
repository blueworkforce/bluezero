#ifndef B0__LOGGER__LOGGER_H__INCLUDED
#define B0__LOGGER__LOGGER_H__INCLUDED

#include <b0/utils/protobufhelpers.h>
#include <b0/logger/interface.h>

#include <string>
#include <sstream>
#include <boost/thread.hpp>
#include <boost/format.hpp>

namespace b0
{

class Node;

template<typename TMsg, bool notifyGraph>
class Publisher;

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
    //! The publisher where LogEntry es will be published
    Publisher<b0::logger_msgs::LogEntry, false> pub_;
};

} // namespace logger

} // namespace b0

#endif // B0__LOGGER__LOGGER_H__INCLUDED

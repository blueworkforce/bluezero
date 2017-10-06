#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED

#include "logger.pb.h"
#include <b0/protobufhelpers.hpp>

#include <string>
#include <sstream>
#include <boost/thread.hpp>
#include <boost/format.hpp>

namespace b0
{

/*!
 * \brief Base class to add logging functionalities to nodes
 */
class LogInterface
{
public:
    /*!
     * \brief Log a message to the remote logger, with the default level (INFO)
     */
    virtual void log(std::string message)
    {
        log(INFO, message);
    }

    /*!
     * \brief Log a message to the remote logger, at default level, using a format string
     */
    template<typename... Arguments>
    void log(std::string const &fmt, Arguments&&... args)
    {
        boost::format format(fmt);
        log_helper(INFO, format, std::forward<Arguments>(args)...);
    }

    /*!
     * \brief Log a message to the remote logger, with a specified level
     */
    virtual void log(b0::logger_msgs::LogLevel level, std::string message) = 0;

    /*!
     * \brief Log a message using a format string
     */
    template<typename... Arguments>
    void log(b0::logger_msgs::LogLevel level, std::string const &fmt, Arguments&&... args)
    {
        boost::format format(fmt);
        log_helper(level, format, std::forward<Arguments>(args)...);
    }

protected:
    //! \cond HIDDEN_SYMBOLS

    virtual void log_helper(b0::logger_msgs::LogLevel level, boost::format &format)
    {
        return log(level, format.str());
    }

    template<class T, class... Args>
    void log_helper(b0::logger_msgs::LogLevel level, boost::format &format, T &&t, Args&&... args)
    {
        return log_helper(level, format % std::forward<T>(t), std::forward<Args>(args)...);
    }

    //! \endcond

public:
    //! \brief The most verbose level
    const b0::logger_msgs::LogLevel TRACE = b0::logger_msgs::LogLevel::TRACE;

    //! \brief Slightly less verbose
    const b0::logger_msgs::LogLevel DEBUG = b0::logger_msgs::LogLevel::DEBUG;

    //! \brief For information messages
    const b0::logger_msgs::LogLevel INFO  = b0::logger_msgs::LogLevel::INFO;

    //! \brief For warning messages
    const b0::logger_msgs::LogLevel WARN  = b0::logger_msgs::LogLevel::WARN;

    //! \brief For error messages
    const b0::logger_msgs::LogLevel ERROR = b0::logger_msgs::LogLevel::ERROR;

    //! \brief For fatal messages (i.e. node should quit after that)
    const b0::logger_msgs::LogLevel FATAL = b0::logger_msgs::LogLevel::FATAL;
};

/*!
 * \brief A logger which prints messages to local console.
 */
template<typename T>
class LocalLogger : public LogInterface
{
public:
    using LogInterface::log;

    //! Constructor
    LocalLogger(T *named_object)
        : named_object_(*named_object)
    {
    }

    //! Destructor
    virtual ~LocalLogger()
    {
    }

    //! Return the name of the object being logged
    std::string getName() const
    {
        return named_object_.getName();
    }

    virtual void log(b0::logger_msgs::LogLevel level, std::string message) override
    {
        LevelInfo info = levelInfo(level);
        std::string name = named_object_.getName();
        if(name != "") name = "[" + name + "] ";
        std::stringstream ss;
        ss << info.ansiEscape() << name << info.levelStr << ": " << message << info.ansiReset() << std::endl;
        std::cout << ss.str();
    }

    struct LevelInfo
    {
        std::string levelStr;
        int attr;
        int fg;
        int bg;

        std::string ansiEscape()
        {
            static boost::format fmt("\x1b[%d;%dm");
            return (fmt % attr % fg).str();
        }

        std::string ansiReset()
        {
            return "\x1b[0m";
        }
    };

    /*!
     * \brief Return meta-information for the specified level
     */
    virtual LevelInfo levelInfo(b0::logger_msgs::LogLevel level)
    {
        switch(level)
        {
            case b0::logger_msgs::LogLevel::TRACE: return {"TRACE", 0, 0x1e, 0}; break;
            case b0::logger_msgs::LogLevel::DEBUG: return {"DEBUG", 1, 0x1e, 0}; break;
            case b0::logger_msgs::LogLevel::INFO:  return {"INFO",  1, 0x25, 0}; break;
            case b0::logger_msgs::LogLevel::WARN:  return {"WARN",  0, 0x21, 0}; break;
            case b0::logger_msgs::LogLevel::ERROR: return {"ERROR", 0, 0x1f, 0}; break;
            case b0::logger_msgs::LogLevel::FATAL: return {"FATAL", 7, 0x1f, 0}; break;
        }
        return {"?????", 1, 0x1e, 0};
    }

protected:
    //! An object with a std::string getName() method
    T &named_object_;
};

/*!
 * \brief A subclass LocalLogger<T> which also sends log messages remotely, via a ZeroMQ PUB socket
 */
template<typename T>
class Logger : public LocalLogger<T>
{
public:
    using LocalLogger<T>::log;
    using LocalLogger<T>::getName;

    //! Construct a Logger for the given named object
    Logger(T *named_object)
        : LocalLogger<T>(named_object),
          pub_socket_(named_object->getZMQContext(), ZMQ_PUB)
    {
    }

    virtual ~Logger()
    {
    }

    //! Connect the underlying ZeroMQ PUB socket to the given address
    void connect(std::string addr)
    {
        pub_socket_.connect(addr);
    }

    void log(b0::logger_msgs::LogLevel level, std::string message) override
    {
        std::string name = getName();

        LocalLogger<T>::log(level, message);

        b0::logger_msgs::LogEntry e;
        e.set_node_name(name);
        e.set_level(level);
        e.set_msg(message);
        ::s_sendmore(pub_socket_, std::string("log"));
        ::s_send(pub_socket_, e);
    }

protected:
    //! The ZeroMQ PUB socket where LogEntry es will be published
    zmq::socket_t pub_socket_;
};

} // namespace b0

#endif // LOGGER_H_INCLUDED

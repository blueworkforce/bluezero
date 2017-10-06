#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED

#include "logger.pb.h"
#include <b0/protobufhelpers.hpp>

#include <string>
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
    //! Log a message to the logger (will publish a message)
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
        int style = levelStyle(level);
        std::string colStart = "", colEnd = "";
        if(true)
        {
            int attr = (style & 0x00ff0000) >> 16;
            int col = (style & 0x0000ff00) >> 8;
            colStart = (boost::format("\x1b[%d;%dm") % attr % col).str();
            colEnd = "\x1b[0m";
        }

        std::string name = named_object_.getName();
        static boost::format fmt("%s%s[%s] %s%s");
        if(name != "") name = "[" + name + "] ";
        std::string output = (fmt % colStart % name % levelString(level) % message % colEnd).str();
        std::cout << output << std::endl;
    }

    /*!
     * \brief Return a string for this level
     */
    virtual std::string levelString(b0::logger_msgs::LogLevel level)
    {
        switch(level)
        {
            case b0::logger_msgs::LogLevel::TRACE: return "TRACE"; break;
            case b0::logger_msgs::LogLevel::DEBUG: return "DEBUG"; break;
            case b0::logger_msgs::LogLevel::INFO:  return "INFO";  break;
            case b0::logger_msgs::LogLevel::WARN:  return "WARN";  break;
            case b0::logger_msgs::LogLevel::ERROR: return "ERROR"; break;
            case b0::logger_msgs::LogLevel::FATAL: return "FATAL"; break;
        }
        return "?????";
    }

    /*!
     * \brief Return a dword for the style of this level
     *
     * The dword's most significant byte is not used.
     * The next byte is used for the attribute.
     * The next byte is used for the foreground color.
     * The next byte is used for the background color.
     */
    virtual int levelStyle(b0::logger_msgs::LogLevel level)
    {
        switch(level)
        {
            case b0::logger_msgs::LogLevel::TRACE: return 0x00001e00; break;
            case b0::logger_msgs::LogLevel::DEBUG: return 0x00011e00; break;
            case b0::logger_msgs::LogLevel::INFO:  return 0x00012500; break;
            case b0::logger_msgs::LogLevel::WARN:  return 0x00002100; break;
            case b0::logger_msgs::LogLevel::ERROR: return 0x00001f00; break;
            case b0::logger_msgs::LogLevel::FATAL: return 0x00071f00; break;
        }
        return 0;
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

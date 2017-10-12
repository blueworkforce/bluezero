#include <b0/logger/logger.h>
#include <b0/node.h>

namespace b0
{

namespace logger
{

void LogInterface::log(std::string message)
{
    log(INFO, message);
}

void LogInterface::log_helper(b0::logger_msgs::LogLevel level, boost::format &format)
{
    return log(level, format.str());
}

LocalLogger::LocalLogger(b0::Node *node)
    : node_(*node)
{
}

LocalLogger::~LocalLogger()
{
}

void LocalLogger::log(b0::logger_msgs::LogLevel level, std::string message)
{
    LevelInfo info = levelInfo(level);
    std::string name = node_.getName();
    if(name != "") name = "[" + name + "] ";
    std::stringstream ss;
    ss << info.ansiEscape() << name << info.levelStr << ": " << message << info.ansiReset() << std::endl;
    std::cout << ss.str();
}

std::string LocalLogger::LevelInfo::ansiEscape()
{
    static boost::format fmt("\x1b[%d;%dm");
    return (fmt % attr % fg).str();
}

std::string LocalLogger::LevelInfo::ansiReset()
{
    return "\x1b[0m";
}

LocalLogger::LevelInfo LocalLogger::levelInfo(b0::logger_msgs::LogLevel level)
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

Logger::Logger(b0::Node *node)
    : LocalLogger(node),
      pub_socket_(node->getZMQContext(), ZMQ_PUB)
{
}

Logger::~Logger()
{
}

void Logger::connect(std::string addr)
{
    pub_socket_.connect(addr);
}

void Logger::log(b0::logger_msgs::LogLevel level, std::string message)
{
    std::string name = node_.getName();

    LocalLogger::log(level, message);

    b0::logger_msgs::LogEntry e;
    e.set_node_name(name);
    e.set_level(level);
    e.set_msg(message);
    ::s_sendmore(pub_socket_, std::string("log"));
    ::s_send(pub_socket_, e);
}

} // namespace logger

} // namespace b0


#include <b0/logger/logger.h>
#include <b0/node.h>

#include "logger.pb.h"

namespace b0
{

namespace logger
{

void LogInterface::log(std::string message)
{
    log(INFO, message);
}

void LogInterface::log_helper(LogLevel level, boost::format &format)
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

void LocalLogger::log(LogLevel level, std::string message)
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

LocalLogger::LevelInfo LocalLogger::levelInfo(LogLevel level)
{
    switch(level)
    {
        case LogLevel::TRACE: return {"TRACE", 0, 0x1e, 0}; break;
        case LogLevel::DEBUG: return {"DEBUG", 1, 0x1e, 0}; break;
        case LogLevel::INFO:  return {"INFO",  1, 0x25, 0}; break;
        case LogLevel::WARN:  return {"WARN",  0, 0x21, 0}; break;
        case LogLevel::ERROR: return {"ERROR", 0, 0x1f, 0}; break;
        case LogLevel::FATAL: return {"FATAL", 7, 0x1f, 0}; break;
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

void Logger::log(LogLevel level, std::string message)
{
    std::string name = node_.getName();

    LocalLogger::log(level, message);

    b0::logger_msgs::LogEntry e;
    e.set_node_name(name);
    switch(level)
    {
        case LogLevel::TRACE: e.set_level(b0::logger_msgs::TRACE); break;
        case LogLevel::DEBUG: e.set_level(b0::logger_msgs::DEBUG); break;
        case LogLevel::INFO:  e.set_level(b0::logger_msgs::INFO);  break;
        case LogLevel::WARN:  e.set_level(b0::logger_msgs::WARN);  break;
        case LogLevel::ERROR: e.set_level(b0::logger_msgs::ERROR); break;
        case LogLevel::FATAL: e.set_level(b0::logger_msgs::FATAL); break;
    }
    e.set_msg(message);
    ::s_sendmore(pub_socket_, std::string("log"));
    ::s_send(pub_socket_, e);
}

} // namespace logger

} // namespace b0


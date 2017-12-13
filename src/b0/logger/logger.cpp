#include <b0/publisher.h>
#include <b0/logger/logger.h>
#include <b0/node.h>
#include <b0/utils/thread_name.h>

#include "resolver.pb.h"
#include "logger.pb.h"

namespace b0
{

namespace logger
{

void LogInterface::log_helper(LogLevel level, boost::format &format) const
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

void LocalLogger::log(LogLevel level, std::string message) const
{
    LevelInfo info = levelInfo(level);
    std::string name = node_.getName();
    std::stringstream ss;
    ss << info.ansiEscape();
    if(!name.empty())
        ss << "[" << name << "] ";
    ss << "{" << get_thread_name() << "} ";
    ss << info.levelStr << ": " << message << info.ansiReset() << std::endl;
    std::cout << ss.str();
}

std::string LocalLogger::LevelInfo::ansiEscape() const
{
    boost::format fmt("\x1b[%d;%dm");
    return (fmt % attr % fg).str();
}

std::string LocalLogger::LevelInfo::ansiReset() const
{
    return "\x1b[0m";
}

LocalLogger::LevelInfo LocalLogger::levelInfo(LogLevel level) const
{
    switch(level)
    {
        case LogLevel::trace: return {"TRACE", 0, 0x1e, 0}; break;
        case LogLevel::debug: return {"DEBUG", 1, 0x1e, 0}; break;
        case LogLevel::info:  return {"INFO",  1, 0x25, 0}; break;
        case LogLevel::warn:  return {"WARN",  0, 0x21, 0}; break;
        case LogLevel::error: return {"ERROR", 0, 0x1f, 0}; break;
        case LogLevel::fatal: return {"FATAL", 7, 0x1f, 0}; break;
    }
    return {"?????", 1, 0x1e, 0};
}

Logger::Logger(b0::Node *node)
    : LocalLogger(node),
      pub_(node, "log", false, false)
{
}

Logger::~Logger()
{
}

void Logger::connect(std::string addr)
{
    pub_.setRemoteAddress(addr);
    pub_.init();
}

void Logger::log(LogLevel level, std::string message) const
{
    LocalLogger::log(level, message);

    remoteLog(level, message);
}

void Logger::remoteLog(LogLevel level, std::string message) const
{
    std::string name = node_.getName();

    b0::logger_msgs::LogEntry e;
    e.set_node_name(name);
    switch(level)
    {
        case LogLevel::trace: e.set_level(b0::logger_msgs::trace); break;
        case LogLevel::debug: e.set_level(b0::logger_msgs::debug); break;
        case LogLevel::info:  e.set_level(b0::logger_msgs::info);  break;
        case LogLevel::warn:  e.set_level(b0::logger_msgs::warn);  break;
        case LogLevel::error: e.set_level(b0::logger_msgs::error); break;
        case LogLevel::fatal: e.set_level(b0::logger_msgs::fatal); break;
    }
    e.set_msg(message);
    pub_.publish(e);
}

} // namespace logger

} // namespace b0


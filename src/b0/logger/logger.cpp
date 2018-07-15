#include <b0/protobuf/publisher.h>
#include <b0/logger/logger.h>
#include <b0/node.h>
#include <b0/exception/argument_error.h>
#include <b0/utils/thread_name.h>
#include <b0/utils/env.h>

#include <iostream>
#include <iomanip>
#include <chrono>

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

LogInterface::LogLevel LocalLogger::defaultOutputLevel_ = LogLevel::warn;

LocalLogger::LocalLogger(b0::Node *node)
    : node_(*node),
      color_(false)
{
    std::string log_level = b0::env::get("B0_CONSOLE_LOGLEVEL");
    if(log_level == "");
    else if(log_level == "trace") defaultOutputLevel_ = LogLevel::trace;
    else if(log_level == "debug") defaultOutputLevel_ = LogLevel::debug;
    else if(log_level == "info")  defaultOutputLevel_ = LogLevel::info;
    else if(log_level == "warn")  defaultOutputLevel_ = LogLevel::warn;
    else if(log_level == "error") defaultOutputLevel_ = LogLevel::error;
    else if(log_level == "fatal") defaultOutputLevel_ = LogLevel::fatal;
    else throw exception::ArgumentError(log_level, "log level");
    outputLevel_ = defaultOutputLevel_;

    std::string term = b0::env::get("TERM");
    if(term == "xterm-color" || term == "xterm-256color") color_ = true;
}

LocalLogger::~LocalLogger()
{
}

void LocalLogger::log(LogLevel level, std::string message) const
{
    if(level < outputLevel_) return;

    LevelInfo info = levelInfo(level);
    std::string name = node_.getName();
    std::stringstream ss;
    if(color_) ss << info.ansiEscape();

    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S ");

    if(!name.empty()) ss << "[" << name << "] ";

    ss << info.levelStr << ": ";

    ss << message;

    if(color_) ss << info.ansiReset();

    std::cout << ss.str() << std::endl;
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

struct LoggerPrivate
{
    LoggerPrivate(Node *node)
        : pub_(node, "log", false, false)
    {
    }

    protobuf::Publisher<b0::logger_msgs::LogEntry> pub_;
};

Logger::Logger(b0::Node *node)
    : LocalLogger(node),
      private_(new LoggerPrivate(node))
{
}

Logger::~Logger()
{
}

void Logger::connect(std::string addr)
{
    private_->pub_.setRemoteAddress(addr);
    private_->pub_.init();
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
    e.set_time_usec(node_.timeUSec());
    private_->pub_.publish(e);
}

} // namespace logger

} // namespace b0


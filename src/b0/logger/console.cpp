#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <b0/utils/protobufhelpers.hpp>
#include <b0/logger/logger.h>
#include <b0/logger/console.h>

namespace b0
{

namespace logger
{

Console::Console()
    : Node("logger_console"),
      sub_(this, "log", &Console::onLogMessage),
      dummy_logger_(this)
{
}

Console::~Console()
{
}

void Console::onLogMessage(std::string topic, const b0::logger_msgs::LogEntry &entry)
{
    LogLevel level = LogLevel::INFO;
    switch(entry.level())
    {
    case b0::logger_msgs::TRACE: level = LogLevel::TRACE; break;
    case b0::logger_msgs::DEBUG: level = LogLevel::DEBUG; break;
    case b0::logger_msgs::INFO:  level = LogLevel::INFO;  break;
    case b0::logger_msgs::WARN:  level = LogLevel::WARN;  break;
    case b0::logger_msgs::ERROR: level = LogLevel::ERROR; break;
    case b0::logger_msgs::FATAL: level = LogLevel::FATAL; break;
    }
    LocalLogger::LevelInfo info = dummy_logger_.levelInfo(level);
    std::cout << info.ansiEscape() << "[" << entry.node_name() << "] " << info.levelStr << ": " << entry.msg() << info.ansiReset() << std::endl;
}

} // namespace logger

} // namespace b0

int main()
{
    b0::logger::Console console;
    console.init();
    console.spin();
    return 0;
}


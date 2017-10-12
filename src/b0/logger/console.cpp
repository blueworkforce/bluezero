#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <b0/protobufhelpers.hpp>
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
    LocalLogger::LevelInfo info = dummy_logger_.levelInfo(entry.level());
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


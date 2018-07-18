#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <b0/publisher.h>
#include <b0/logger/logger.h>
#include <b0/logger/console.h>

namespace b0
{

namespace logger
{

Console::Console()
    : Node("logger_console"),
      sub_(this, "log", boost::bind(&Console::onLogMessage, this, _1)),
      dummy_logger_(this)
{
}

Console::~Console()
{
}

void Console::onLogMessage(const std::string &msg)
{
    b0::message::LogEntry entry;
    entry.parseFromString(msg);
    LocalLogger::LevelInfo info = dummy_logger_.levelInfo(entry.level);
    std::cout << info.ansiEscape() << "[" << entry.node_name << "] " << info.levelStr << ": " << entry.message << info.ansiReset() << std::endl;
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


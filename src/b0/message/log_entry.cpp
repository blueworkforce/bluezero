#include <b0/message/log_entry.h>

namespace b0
{

namespace message
{

std::string LogEntry::type() const
{
    return "LogEntry";
}

void LogEntry::serialize(serialization::MessageFields &fields) const
{
    fields.map("node_name", &node_name);
    fields.map("level", reinterpret_cast<const int*>(&level));
    fields.map("message", &message);
    fields.map("time_usec", &time_usec);
}

} // namespace message

} // namespace b0


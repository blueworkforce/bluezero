#include <b0/message/heartbeat_response.h>

namespace b0
{

namespace message
{

std::string HeartbeatResponse::type() const
{
    return "HeartbeatResponse";
}

void HeartbeatResponse::serialize(serialization::MessageFields &fields) const
{
    fields.map("ok", &ok);
    fields.map("time_usec", &time_usec);
}

} // namespace message

} // namespace b0


#include <b0/message/heartbeat_request.h>

namespace b0
{

namespace message
{

std::string HeartbeatRequest::type() const
{
    return "HeartbeatRequest";
}

void HeartbeatRequest::serialize(serialization::MessageFields &fields) const
{
    fields.map("node_name", &node_name);
}

} // namespace message

} // namespace b0


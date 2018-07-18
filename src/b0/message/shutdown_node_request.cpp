#include <b0/message/shutdown_node_request.h>

namespace b0
{

namespace message
{

std::string ShutdownNodeRequest::type() const
{
    return "ShutdownNodeRequest";
}

void ShutdownNodeRequest::serialize(serialization::MessageFields &fields) const
{
    fields.map("node_name", &node_name);
}

} // namespace message

} // namespace b0


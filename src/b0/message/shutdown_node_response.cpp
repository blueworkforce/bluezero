#include <b0/message/shutdown_node_response.h>

namespace b0
{

namespace message
{

std::string ShutdownNodeResponse::type() const
{
    return "ShutdownNodeResponse";
}

void ShutdownNodeResponse::serialize(serialization::MessageFields &fields) const
{
    fields.map("ok", &ok);
}

} // namespace message

} // namespace b0


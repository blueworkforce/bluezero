#include <b0/message/announce_node_request.h>

namespace b0
{

namespace message
{

std::string AnnounceNodeRequest::type() const
{
    return "AnnounceNodeRequest";
}

void AnnounceNodeRequest::serialize(serialization::MessageFields &fields) const
{
    fields.map("node_name", &node_name);
}

} // namespace message

} // namespace b0


#include <b0/message/announce_node_response.h>

namespace b0
{

namespace message
{

std::string AnnounceNodeResponse::type() const
{
    return "AnnounceNodeResponse";
}

void AnnounceNodeResponse::serialize(serialization::MessageFields &fields) const
{
    fields.map("ok", &ok);
    fields.map("node_name", &node_name);
    fields.map("xsub_sock_addr", &xsub_sock_addr);
    fields.map("xpub_sock_addr", &xpub_sock_addr);
}

} // namespace message

} // namespace b0


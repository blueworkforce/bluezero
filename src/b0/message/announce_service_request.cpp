#include <b0/message/announce_service_request.h>

namespace b0
{

namespace message
{

std::string AnnounceServiceRequest::type() const
{
    return "AnnounceServiceRequest";
}

void AnnounceServiceRequest::serialize(serialization::MessageFields &fields) const
{
    fields.map("node_name", &node_name);
    fields.map("service_name", &service_name);
    fields.map("sock_addr", &sock_addr);
}

} // namespace message

} // namespace b0


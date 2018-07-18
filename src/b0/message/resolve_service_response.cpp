#include <b0/message/resolve_service_response.h>

namespace b0
{

namespace message
{

std::string ResolveServiceResponse::type() const
{
    return "ResolveServiceResponse";
}

void ResolveServiceResponse::serialize(serialization::MessageFields &fields) const
{
    fields.map("ok", &ok);
    fields.map("sock_addr", &sock_addr);
}

} // namespace message

} // namespace b0


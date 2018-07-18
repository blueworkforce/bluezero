#include <b0/message/resolve_service_request.h>

namespace b0
{

namespace message
{

std::string ResolveServiceRequest::type() const
{
    return "ResolveServiceRequest";
}

void ResolveServiceRequest::serialize(serialization::MessageFields &fields) const
{
    fields.map("service_name", &service_name);
}

} // namespace message

} // namespace b0


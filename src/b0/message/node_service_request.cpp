#include <b0/message/node_service_request.h>

namespace b0
{

namespace message
{

std::string NodeServiceRequest::type() const
{
    return "NodeServiceRequest";
}

void NodeServiceRequest::serialize(serialization::MessageFields &fields) const
{
    fields.map("node_name", &node_name);
    fields.map("service_name", &service_name);
    fields.map("reverse", &reverse);
    fields.map("active", &active);
}

} // namespace message

} // namespace b0


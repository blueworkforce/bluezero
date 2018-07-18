#include <b0/message/node_service_response.h>

namespace b0
{

namespace message
{

std::string NodeServiceResponse::type() const
{
    return "NodeServiceResponse";
}

void NodeServiceResponse::serialize(serialization::MessageFields &fields) const
{
}

} // namespace message

} // namespace b0


#include <b0/message/get_graph_request.h>

namespace b0
{

namespace message
{

std::string GetGraphRequest::type() const
{
    return "GetGraphRequest";
}

void GetGraphRequest::serialize(serialization::MessageFields &fields) const
{
}

} // namespace message

} // namespace b0


#include <b0/message/get_graph_response.h>

namespace b0
{

namespace message
{

std::string GetGraphResponse::type() const
{
    return "GetGraphResponse";
}

void GetGraphResponse::serialize(serialization::MessageFields &fields) const
{
    fields.map("graph", &graph);
}

} // namespace message

} // namespace b0


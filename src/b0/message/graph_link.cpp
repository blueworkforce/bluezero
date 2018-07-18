#include <b0/message/graph_link.h>

namespace b0
{

namespace message
{

std::string GraphLink::type() const
{
    return "GraphLink";
}

void GraphLink::serialize(serialization::MessageFields &fields) const
{
    fields.map("node_name", &node_name);
    fields.map("other_name", &other_name);
    fields.map("reversed", &reversed);
}

} // namespace message

} // namespace b0


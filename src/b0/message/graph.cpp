#include <b0/message/graph.h>

namespace b0
{

namespace message
{

std::string Graph::type() const
{
    return "Graph";
}

void Graph::serialize(serialization::MessageFields &fields) const
{
    fields.map("nodes", &nodes);
    fields.map("node_topic", &node_topic);
    fields.map("node_service", &node_service);
}

} // namespace message

} // namespace b0


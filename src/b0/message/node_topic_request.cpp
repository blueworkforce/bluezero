#include <b0/message/node_topic_request.h>

namespace b0
{

namespace message
{

std::string NodeTopicRequest::type() const
{
    return "NodeTopicRequest";
}

void NodeTopicRequest::serialize(serialization::MessageFields &fields) const
{
    fields.map("node_name", &node_name);
    fields.map("topic_name", &topic_name);
    fields.map("reverse", &reverse);
    fields.map("active", &active);
}

} // namespace message

} // namespace b0


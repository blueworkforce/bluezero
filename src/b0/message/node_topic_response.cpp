#include <b0/message/node_topic_response.h>

namespace b0
{

namespace message
{

std::string NodeTopicResponse::type() const
{
    return "NodeTopicResponse";
}

void NodeTopicResponse::serialize(serialization::MessageFields &fields) const
{
}

} // namespace message

} // namespace b0


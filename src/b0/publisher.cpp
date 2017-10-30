#include <b0/publisher.h>

namespace b0
{

AbstractPublisher::AbstractPublisher(Node *node, std::string topic)
    : node_(*node),
      pub_socket_(node_.getZMQContext(), ZMQ_PUB),
      topic_name_(topic)
{
    node_.addPublisher(this);
}

AbstractPublisher::~AbstractPublisher()
{
    node_.removePublisher(this);
}

void AbstractPublisher::init()
{
    pub_socket_.connect(node_.getXSUBSocketAddress());
}

void AbstractPublisher::cleanup()
{
    pub_socket_.disconnect(node_.getXSUBSocketAddress());
}

std::string AbstractPublisher::getTopicName()
{
    return topic_name_;
}

} // namespace b0


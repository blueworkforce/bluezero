#include <b0/publisher.h>
#include <b0/node.h>

namespace b0
{

AbstractPublisher::AbstractPublisher(Node *node, std::string topic, bool managed)
    : node_(*node),
      topic_name_(topic),
      managed_(managed),
      pub_socket_(node_.getZMQContext(), ZMQ_PUB)
{
    if(managed_)
        node_.addPublisher(this);
}

AbstractPublisher::~AbstractPublisher()
{
    if(managed_)
        node_.removePublisher(this);
}

void AbstractPublisher::setRemoteAddress(std::string addr)
{
    remote_addr_ = addr;
}

void AbstractPublisher::init()
{
    if(remote_addr_.empty())
        remote_addr_ = node_.getXSUBSocketAddress();
    pub_socket_.connect(remote_addr_);
}

void AbstractPublisher::cleanup()
{
    pub_socket_.disconnect(remote_addr_);
}

std::string AbstractPublisher::getTopicName()
{
    return topic_name_;
}

bool AbstractPublisher::writeRaw(const std::string &topic, const std::string &msg)
{
    ::s_sendmore(pub_socket_, topic);
    ::s_send(pub_socket_, msg);
    return true;
}

template<>
bool Publisher<std::string, true>::write(const std::string &topic, const std::string &msg)
{
    return AbstractPublisher::writeRaw(topic, msg);
}

} // namespace b0


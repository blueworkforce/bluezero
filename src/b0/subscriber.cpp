#include <b0/subscriber.h>

namespace b0
{

AbstractSubscriber::AbstractSubscriber(Node *node, std::string topic)
    : node_(*node),
      sub_socket_(node_.getZMQContext(), ZMQ_SUB),
      topic_name_(topic)
{
    node_.addSubscriber(this);
}

AbstractSubscriber::~AbstractSubscriber()
{
    node_.removeSubscriber(this);
}

void AbstractSubscriber::init()
{
    sub_socket_.connect(node_.getXPUBSocketAddress());
    sub_socket_.setsockopt(ZMQ_SUBSCRIBE, topic_name_.data(), topic_name_.size());
}

void AbstractSubscriber::cleanup()
{
    sub_socket_.setsockopt(ZMQ_UNSUBSCRIBE, topic_name_.data(), topic_name_.size());
    sub_socket_.disconnect(node_.getXPUBSocketAddress());
}

std::string AbstractSubscriber::getTopicName()
{
    return topic_name_;
}

bool AbstractSubscriber::poll(long timeout)
{
#ifdef __GNUC__
    zmq::pollitem_t items[] = {{static_cast<void*>(sub_socket_), 0, ZMQ_POLLIN, 0}};
#else
    zmq::pollitem_t items[] = {{sub_socket_, 0, ZMQ_POLLIN, 0}};
#endif
    zmq::poll(&items[0], sizeof(items) / sizeof(items[0]), timeout);
    return items[0].revents & ZMQ_POLLIN;
}

bool AbstractSubscriber::readRaw(std::string &topic, std::string &msg)
{
    topic = ::s_recv(sub_socket_);
    msg = ::s_recv(sub_socket_);
    return true;
}

template<>
bool Subscriber<std::string, true>::read(std::string &topic, std::string &msg)
{
    return AbstractSubscriber::readRaw(topic, msg);
}

} // namespace b0


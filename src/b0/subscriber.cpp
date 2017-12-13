#include <b0/subscriber.h>
#include <b0/node.h>

#include "resolver.pb.h"
#include "logger.pb.h"

namespace b0
{

AbstractSubscriber::AbstractSubscriber(Node *node, std::string topic, bool managed, bool notify_graph)
    : socket::Socket(node, zmq::socket_type::sub, topic, managed),
      notify_graph_(notify_graph)
{
    setHasHeader(true);
}

AbstractSubscriber::~AbstractSubscriber()
{
}

void AbstractSubscriber::log(LogLevel level, std::string message) const
{
    boost::format fmt("Subscriber(%s): %s");
    Socket::log(level, (fmt % name_ % message).str());
}

void AbstractSubscriber::init()
{
    if(remote_addr_.empty())
        remote_addr_ = node_.getXPUBSocketAddress();
    connect();

    if(notify_graph_)
        node_.resolverClient().notifyTopic(name_, true, true);
}

void AbstractSubscriber::cleanup()
{
    disconnect();

    if(notify_graph_)
        node_.resolverClient().notifyTopic(name_, true, false);
}

std::string AbstractSubscriber::getTopicName()
{
    return name_;
}

void AbstractSubscriber::connect()
{
    log(TRACE, "Connecting to %s...", remote_addr_);
    socket_.connect(remote_addr_);
    socket_.setsockopt(ZMQ_SUBSCRIBE, name_.data(), name_.size());
}

void AbstractSubscriber::disconnect()
{
    log(TRACE, "Disconnecting from %s...", remote_addr_);
    socket_.setsockopt(ZMQ_UNSUBSCRIBE, name_.data(), name_.size());
    socket_.disconnect(remote_addr_);
}

} // namespace b0


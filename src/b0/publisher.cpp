#include <b0/publisher.h>
#include <b0/node.h>

#include "resolver.pb.h"
#include "logger.pb.h"

#include <zmq.hpp>

namespace b0
{

AbstractPublisher::AbstractPublisher(Node *node, std::string topic, bool managed, bool notify_graph)
    : socket::Socket(node, ZMQ_PUB, topic, managed),
      notify_graph_(notify_graph)
{
    setHasHeader(true);
}

AbstractPublisher::~AbstractPublisher()
{
}

void AbstractPublisher::log(LogLevel level, std::string message) const
{
    boost::format fmt("Publisher(%s): %s");
    Socket::log(level, (fmt % name_ % message).str());
}

void AbstractPublisher::init()
{
    if(remote_addr_.empty())
        remote_addr_ = node_.getXSUBSocketAddress();
    connect();

    if(notify_graph_)
        node_.notifyTopic(name_, false, true);
}

void AbstractPublisher::cleanup()
{
    disconnect();

    if(notify_graph_)
        node_.notifyTopic(name_, false, false);
}

std::string AbstractPublisher::getTopicName()
{
    return name_;
}

void AbstractPublisher::connect()
{
    log(trace, "Connecting to %s...", remote_addr_);
    Socket::connect(remote_addr_);
}

void AbstractPublisher::disconnect()
{
    log(trace, "Disconnecting from %s...", remote_addr_);
    Socket::disconnect(remote_addr_);
}

} // namespace b0


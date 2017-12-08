#include <b0/publisher.h>
#include <b0/node.h>
#include <b0/envelope.h>

namespace b0
{

AbstractPublisher::AbstractPublisher(Node *node, std::string topic, bool managed)
    : socket::Socket(node, zmq::socket_type::pub, topic, managed)
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
}

void AbstractPublisher::cleanup()
{
    disconnect();
}

std::string AbstractPublisher::getTopicName()
{
    return name_;
}

void AbstractPublisher::connect()
{
    log(TRACE, "Connecting to %s...", remote_addr_);
    socket_.connect(remote_addr_);
}

void AbstractPublisher::disconnect()
{
    log(TRACE, "Disconnecting from %s...", remote_addr_);
    socket_.disconnect(remote_addr_);
}

} // namespace b0


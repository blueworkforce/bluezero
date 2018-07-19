#include <b0/subscriber.h>
#include <b0/node.h>

#include <zmq.hpp>

namespace b0
{

Subscriber::Subscriber(Node *node, std::string topic_name, CallbackWithoutType callback, bool managed, bool notify_graph)
    : Socket(node, ZMQ_SUB, topic_name, managed),
      notify_graph_(notify_graph),
      callback_(callback)
{
    setHasHeader(true);
}

Subscriber::Subscriber(Node *node, std::string topic_name, CallbackWithType callback, bool managed, bool notify_graph)
    : Socket(node, ZMQ_SUB, topic_name, managed),
      notify_graph_(notify_graph),
      callback_with_type_(callback)
{
    setHasHeader(true);
}

Subscriber::Subscriber(Node *node, std::string topic_name, CallbackRawParts callback, bool managed, bool notify_graph)
    : Socket(node, ZMQ_SUB, topic_name, managed),
      notify_graph_(notify_graph),
      callback_multipart_(callback)
{
    setHasHeader(true);
}

Subscriber::~Subscriber()
{
}

void Subscriber::log(LogLevel level, std::string message) const
{
    boost::format fmt("Subscriber(%s): %s");
    Socket::log(level, (fmt % name_ % message).str());
}

void Subscriber::init()
{
    if(remote_addr_.empty())
        remote_addr_ = node_.getXPUBSocketAddress();
    connect();

    if(notify_graph_)
        node_.notifyTopic(name_, true, true);
}

void Subscriber::cleanup()
{
    disconnect();

    if(notify_graph_)
        node_.notifyTopic(name_, true, false);
}

void Subscriber::spinOnce()
{
    if(!callback_ && !callback_with_type_ && !callback_multipart_) return;

    while(poll())
    {
        if(callback_)
        {
            std::string msg;
            readRaw(msg);
            callback_(msg);
        }
        if(callback_with_type_)
        {
            std::string msg, type;
            readRaw(msg, type);
            callback_with_type_(msg, type);
        }
        if(callback_multipart_)
        {
            std::vector<b0::message::MessagePart> parts;
            readRaw(parts);
            callback_multipart_(parts);
        }
    }
}

std::string Subscriber::getTopicName()
{
    return name_;
}

void Subscriber::connect()
{
    log(trace, "Connecting to %s...", remote_addr_);
    Socket::connect(remote_addr_);
    std::string subscription("Header: " + name_);
    Socket::setsockopt(ZMQ_SUBSCRIBE, subscription.data(), subscription.size());
}

void Subscriber::disconnect()
{
    log(trace, "Disconnecting from %s...", remote_addr_);
    std::string subscription("Header: " + name_);
    Socket::setsockopt(ZMQ_UNSUBSCRIBE, subscription.data(), subscription.size());
    Socket::disconnect(remote_addr_);
}

} // namespace b0


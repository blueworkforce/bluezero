#include <b0/service_client.h>
#include <b0/node.h>

#include "resolver.pb.h"
#include "logger.pb.h"

#include <zmq.hpp>

namespace b0
{

AbstractServiceClient::AbstractServiceClient(Node *node, std::string service_name, bool managed, bool notify_graph)
    : socket::Socket(node, ZMQ_REQ, service_name, managed),
      notify_graph_(notify_graph)
{
}

AbstractServiceClient::~AbstractServiceClient()
{
}

void AbstractServiceClient::log(LogLevel level, std::string message) const
{
    boost::format fmt("ServiceClient(%s): %s");
    Socket::log(level, (fmt % name_ % message).str());
}

void AbstractServiceClient::init()
{
    resolve();
    connect();

    if(notify_graph_)
        node_.notifyService(name_, true, true);
}

void AbstractServiceClient::cleanup()
{
    disconnect();

    if(notify_graph_)
        node_.notifyService(name_, true, false);
}

std::string AbstractServiceClient::getServiceName()
{
    return name_;
}

void AbstractServiceClient::resolve()
{
    if(!remote_addr_.empty())
    {
        log(debug, "Skipping resolution because remote address (%s) was given", remote_addr_);
        return;
    }

    node_.resolveService(name_, remote_addr_);

    log(trace, "Resolved address: %s", remote_addr_);
}

void AbstractServiceClient::connect()
{
    log(trace, "Connecting to %s...", remote_addr_);
    Socket::connect(remote_addr_);
}

void AbstractServiceClient::disconnect()
{
    log(trace, "Disconnecting from %s...", remote_addr_);
    Socket::disconnect(remote_addr_);
}

} // namespace b0


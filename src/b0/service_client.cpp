#include <b0/service_client.h>
#include <b0/node.h>
#include <b0/envelope.h>

#include "resolver.pb.h"

namespace b0
{

AbstractServiceClient::AbstractServiceClient(Node *node, std::string service_name, bool managed)
    : socket::Socket(node, zmq::socket_type::req, service_name, managed)
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
}

void AbstractServiceClient::cleanup()
{
    disconnect();
}

std::string AbstractServiceClient::getServiceName()
{
    return name_;
}

void AbstractServiceClient::resolve()
{
    if(!remote_addr_.empty())
    {
        log(DEBUG, "Skipping resolution because remote address (%s) was given", remote_addr_);
        return;
    }

    Node::ResolverServiceClient &resolv_cli = node_.resolverClient();

    b0::resolver_msgs::Request rq0;
    b0::resolver_msgs::ResolveServiceRequest &rq = *rq0.mutable_resolve();
    rq.set_service_name(name_);

    b0::resolver_msgs::Response rsp0;
    resolv_cli.call(rq0, rsp0);
    const b0::resolver_msgs::ResolveServiceResponse &rsp = rsp0.resolve();
    remote_addr_ = rsp.sock_addr();
    log(TRACE, "Resolved address: %s", remote_addr_);
}

void AbstractServiceClient::connect()
{
    log(TRACE, "Connecting to %s...", remote_addr_);
    socket_.connect(remote_addr_);
}

void AbstractServiceClient::disconnect()
{
    log(TRACE, "Disconnecting from %s...", remote_addr_);
    socket_.disconnect(remote_addr_);
}

} // namespace b0


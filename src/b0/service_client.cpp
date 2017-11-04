#include <b0/service_client.h>

#include "resolver.pb.h"

namespace b0
{

AbstractServiceClient::AbstractServiceClient(Node *node, std::string service_name)
    : node_(*node),
      service_name_(service_name),
      req_socket_(node_.getZMQContext(), ZMQ_REQ)
{
    node_.addServiceClient(this);
}

AbstractServiceClient::~AbstractServiceClient()
{
    node_.removeServiceClient(this);
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
    return service_name_;
}

void AbstractServiceClient::resolve()
{
    zmq::socket_t &resolv_socket = node_.resolverSocket();

    b0::resolver_msgs::Request rq0;
    b0::resolver_msgs::ResolveServiceRequest &rq = *rq0.mutable_resolve();
    rq.set_service_name(service_name_);
    s_send(resolv_socket, rq0);

    b0::resolver_msgs::Response rsp0;
    s_recv(resolv_socket, rsp0);
    const b0::resolver_msgs::ResolveServiceResponse &rsp = rsp0.resolve();
    remote_addr_ = rsp.sock_addr();
    node_.log(node_.TRACE, "Resolve %s -> %s", service_name_, remote_addr_);
}

void AbstractServiceClient::connect()
{
    node_.log(node_.TRACE, "Connecting to service '%s' (%s)...", service_name_, remote_addr_);
    req_socket_.connect(remote_addr_);
}

void AbstractServiceClient::disconnect()
{
    node_.log(node_.TRACE, "Disconnecting from service '%s' (%s)...", service_name_, remote_addr_);
    req_socket_.disconnect(remote_addr_);
}

bool AbstractServiceClient::writeRaw(const std::string &msg)
{
    ::s_send(req_socket_, msg);
    return true;
}

bool AbstractServiceClient::poll(long timeout)
{
#ifdef __GNUC__
    zmq::pollitem_t items[] = {{static_cast<void*>(req_socket_), 0, ZMQ_POLLIN, 0}};
#else
    zmq::pollitem_t items[] = {{req_socket_, 0, ZMQ_POLLIN, 0}};
#endif
    zmq::poll(&items[0], sizeof(items) / sizeof(items[0]), timeout);
    return items[0].revents & ZMQ_POLLIN;
}

bool AbstractServiceClient::readRaw(std::string &msg)
{
    msg = ::s_recv(req_socket_);
    return true;
}

template<>
bool ServiceClient<std::string, std::string, true>::write(const std::string &req)
{
    return AbstractServiceClient::writeRaw(req);
}

template<>
bool ServiceClient<std::string, std::string, true>::read(std::string &rep)
{
    return AbstractServiceClient::readRaw(rep);
}

} // namespace b0


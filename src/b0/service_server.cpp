#include <b0/service_server.h>

#include "resolver.pb.h"

namespace b0
{

AbstractServiceServer::AbstractServiceServer(Node *node, std::string service_name)
    : node_(*node),
      service_name_(service_name),
      rep_socket_(node_.getZMQContext(), ZMQ_REP),
      bind_addr_(""),
      remote_addr_("")
{
    node_.addServiceServer(this);
}

AbstractServiceServer::~AbstractServiceServer()
{
    node_.removeServiceServer(this);
}

void AbstractServiceServer::init()
{
    bind();
    announce();
}

void AbstractServiceServer::cleanup()
{
    unbind();
}

std::string AbstractServiceServer::getServiceName()
{
    return service_name_;
}

void AbstractServiceServer::bind()
{
    static boost::format fmt("tcp://%s:%d");
    std::string host = node_.hostname();
    int port = node_.freeTCPPort();
    bind_addr_ = (fmt % "*" % port).str();
    remote_addr_ = (fmt % host % port).str();
    rep_socket_.bind(bind_addr_);
    node_.log(node_.DEBUG, "Bound service '%s' to %s", service_name_, bind_addr_);
}

void AbstractServiceServer::unbind()
{
    //rep_socket_.unbind(bind_addr_); // FIXME: causes a zmq error on node shutdown
}

void AbstractServiceServer::announce()
{
    zmq::socket_t &resolv_socket = node_.resolverSocket();

    node_.log(node_.TRACE, "Announcing service '%s' (%s) to resolver...", service_name_, remote_addr_);
    b0::resolver_msgs::Request rq0;
    b0::resolver_msgs::AnnounceServiceRequest &rq = *rq0.mutable_announce_service();
    b0::resolver_msgs::NodeID &node_id = *rq.mutable_node_id();
    node_id.set_host_id(node_.hostname());
    node_id.set_process_id(node_.pid());
    node_id.set_thread_id(node_.threadID());
    rq.set_service_name(service_name_);
    rq.set_sock_addr(remote_addr_);
    s_send(resolv_socket, rq0);

    b0::resolver_msgs::Response rsp0;
    s_recv(resolv_socket, rsp0);
    const b0::resolver_msgs::AnnounceServiceResponse &rsp = rsp0.announce_service();
}

template<>
bool ServiceServer<std::string, std::string, true>::read(std::string &req)
{
    req = ::s_recv(rep_socket_);
    return true;
}

template<>
bool ServiceServer<std::string, std::string, true>::write(const std::string &rep)
{
    ::s_send(rep_socket_, rep);
    return true;
}

} // namespace b0


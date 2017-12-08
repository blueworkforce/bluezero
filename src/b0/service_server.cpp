#include <b0/service_server.h>
#include <b0/node.h>
#include <b0/envelope.h>

#include "resolver.pb.h"

namespace b0
{

AbstractServiceServer::AbstractServiceServer(Node *node, std::string service_name, bool managed)
    : socket::Socket(node, zmq::socket_type::rep, service_name, managed),
      bind_addr_("")
{
}

AbstractServiceServer::~AbstractServiceServer()
{
}

void AbstractServiceServer::log(LogLevel level, std::string message) const
{
    boost::format fmt("ServiceServer(%s): %s");
    Socket::log(level, (fmt % name_ % message).str());
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
    return name_;
}

void AbstractServiceServer::bind()
{
    boost::format fmt("tcp://%s:%d");
    std::string host = node_.hostname();
    int port = node_.freeTCPPort();
    bind_addr_ = (fmt % "*" % port).str();
    remote_addr_ = (fmt % host % port).str();
    socket_.bind(bind_addr_);
    log(DEBUG, "Bound to %s", bind_addr_);
}

void AbstractServiceServer::unbind()
{
    //socket_.unbind(bind_addr_); // FIXME: causes a zmq error on node shutdown
}

void AbstractServiceServer::announce()
{
    Node::ResolverServiceClient &resolv_cli = node_.resolverClient();

    log(TRACE, "Announcing %s to resolver...", remote_addr_);
    b0::resolver_msgs::Request rq0;
    b0::resolver_msgs::AnnounceServiceRequest &rq = *rq0.mutable_announce_service();
    b0::resolver_msgs::NodeID &node_id = *rq.mutable_node_id();
    node_id.set_host_id(node_.hostname());
    node_id.set_process_id(node_.pid());
    node_id.set_thread_id(node_.threadID());
    rq.set_service_name(name_);
    rq.set_sock_addr(remote_addr_);

    b0::resolver_msgs::Response rsp0;
    resolv_cli.call(rq0, rsp0);
    const b0::resolver_msgs::AnnounceServiceResponse &rsp = rsp0.announce_service();
}

void AbstractServiceServer::bind(std::string address)
{
    socket_.bind(address);
    log(DEBUG, "Bound to additional address %s", address);
}

} // namespace b0


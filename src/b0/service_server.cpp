#include <b0/service_server.h>
#include <b0/node.h>
#include <b0/envelope.h>

#include "resolver.pb.h"

namespace b0
{

AbstractServiceServer::AbstractServiceServer(Node *node, std::string service_name, bool managed)
    : node_(*node),
      service_name_(service_name),
      managed_(managed),
      rep_socket_(node_.getZMQContext(), ZMQ_REP),
      bind_addr_(""),
      remote_addr_("")
{
    if(managed_)
        node_.addServiceServer(this);
}

AbstractServiceServer::~AbstractServiceServer()
{
    if(managed_)
        node_.removeServiceServer(this);
}

void AbstractServiceServer::setCompression(std::string algorithm, int level)
{
    compression_algorithm_ = algorithm;
    compression_level_ = level;
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
    boost::format fmt("tcp://%s:%d");
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
    Node::ResolverServiceClient &resolv_cli = node_.resolverClient();

    node_.log(node_.TRACE, "Announcing service '%s' (%s) to resolver...", service_name_, remote_addr_);
    b0::resolver_msgs::Request rq0;
    b0::resolver_msgs::AnnounceServiceRequest &rq = *rq0.mutable_announce_service();
    b0::resolver_msgs::NodeID &node_id = *rq.mutable_node_id();
    node_id.set_host_id(node_.hostname());
    node_id.set_process_id(node_.pid());
    node_id.set_thread_id(node_.threadID());
    rq.set_service_name(service_name_);
    rq.set_sock_addr(remote_addr_);

    b0::resolver_msgs::Response rsp0;
    resolv_cli.call(rq0, rsp0);
    const b0::resolver_msgs::AnnounceServiceResponse &rsp = rsp0.announce_service();
}

void AbstractServiceServer::bind(std::string address)
{
    rep_socket_.bind(address);
}

bool AbstractServiceServer::poll(long timeout)
{
#ifdef __GNUC__
    zmq::pollitem_t items[] = {{static_cast<void*>(rep_socket_), 0, ZMQ_POLLIN, 0}};
#else
    zmq::pollitem_t items[] = {{rep_socket_, 0, ZMQ_POLLIN, 0}};
#endif
    zmq::poll(&items[0], sizeof(items) / sizeof(items[0]), timeout);
    return items[0].revents & ZMQ_POLLIN;
}

bool AbstractServiceServer::readRaw(std::string &msg)
{
    msg = unwrapEnvelope(::s_recv(rep_socket_));
    return true;
}

bool AbstractServiceServer::writeRaw(const std::string &msg)
{
    ::s_send(rep_socket_, wrapEnvelope(msg, compression_algorithm_, compression_level_));
    return true;
}

template<>
bool ServiceServer<std::string, std::string, true>::read(std::string &req)
{
    return AbstractServiceServer::readRaw(req);
}

template<>
bool ServiceServer<std::string, std::string, true>::write(const std::string &rep)
{
    return AbstractServiceServer::writeRaw(rep);
}

} // namespace b0


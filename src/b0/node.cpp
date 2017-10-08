#include <b0/node.h>
#include <b0/publisher.h>
#include <b0/subscriber.h>
#include <b0/service_client.h>
#include <b0/service_server.h>

#include <iostream>
#include <cstdlib>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

namespace b0
{

Node::Node(std::string nodeName)
    : context_(1),
      resolv_socket_(context_, ZMQ_REQ),
      name_(nodeName),
      thread_id_(boost::this_thread::get_id()),
      logger_(this)
{
}

Node::~Node()
{
    // stop the heartbeat_thread so that the last zmq socket will be destroyed
    // and we avoid an unclean exit (zmq::error_t: Context was terminated)
    heartbeat_thread_.interrupt();
    heartbeat_thread_.join();
}

void Node::init()
{
    log(DEBUG, "Initialization.");

    connectToResolver();

    announceNode();

    startHeartbeatThread();

    log(DEBUG, "Initializing publishers...");
    for(auto it = publishers_.begin(); it != publishers_.end(); ++it)
        (*it)->init();
    log(DEBUG, "Initializing service clients...");
    for(auto it = service_clients_.begin(); it != service_clients_.end(); ++it)
        (*it)->init();
    log(DEBUG, "Initializing service servers...");
    for(auto it = service_servers_.begin(); it != service_servers_.end(); ++it)
        (*it)->init();
    log(DEBUG, "Initializing subscribers...");
    for(auto it = subscribers_.begin(); it != subscribers_.end(); ++it)
        (*it)->init();

    log(DEBUG, "Initialization finished.");
}

void Node::log(b0::logger_msgs::LogLevel level, std::string message)
{
    logger_.log(level, message);
}

void Node::connectToResolver()
{
    std::string resolv_addr = resolverAddress();
    log(INFO, "Connecting to resolver at %s...", resolv_addr);
    resolv_socket_.connect(resolv_addr);
}

void Node::startHeartbeatThread()
{
    log(TRACE, "Starting heartbeat thread...");
    heartbeat_thread_ = boost::thread(&Node::heartbeatLoop, this);
}

std::string Node::getName() const
{
    // return this node's name, used to address sockets (together with the socket name).
    // we get this value from the resolver node, during the announceNode() phase.
    return name_;
}

zmq::context_t& Node::getZMQContext()
{
    return context_;
}

std::string Node::getXPUBSocketAddress() const
{
    return xpub_sock_addr_;
}

std::string Node::getXSUBSocketAddress() const
{
    return xsub_sock_addr_;
}

void Node::addPublisher(AbstractPublisher *pub)
{
    publishers_.insert(pub);
}

void Node::removePublisher(AbstractPublisher *pub)
{
    publishers_.erase(pub);
}

void Node::addSubscriber(AbstractSubscriber *sub)
{
    subscribers_.insert(sub);
}

void Node::removeSubscriber(AbstractSubscriber *sub)
{
    subscribers_.erase(sub);
}

void Node::addServiceClient(AbstractServiceClient *cli)
{
    service_clients_.insert(cli);
}

void Node::removeServiceClient(AbstractServiceClient *cli)
{
    service_clients_.erase(cli);
}

void Node::addServiceServer(AbstractServiceServer *srv)
{
    service_servers_.insert(srv);
}

void Node::removeServiceServer(AbstractServiceServer *srv)
{
    service_servers_.erase(srv);
}

void Node::getNodeID(b0::resolver_msgs::NodeID &node_id)
{
    node_id.set_host_id(hostname());
    node_id.set_process_id(::getpid());
    node_id.set_thread_id(boost::lexical_cast<std::string>(thread_id_));
}

std::string Node::hostname()
{
    const char *host_addr = std::getenv("BWF_HOST_ID");
    if(host_addr) return host_addr;
    else return boost::asio::ip::host_name();
}

int Node::freeTCPPort()
{
    // by binding the OS socket to port 0, an available port number will be used
    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::tcp::v4(), 0);
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::socket socket(io_service, ep);
    return socket.local_endpoint().port();
}

std::string Node::freeTCPAddress()
{
    static boost::format fmt("tcp://%s:%d");
    return (fmt % hostname() % freeTCPPort()).str();
}

std::string Node::resolverAddress() const
{
    const char *resolver_addr = std::getenv("BWF_RESOLVER");
    if(resolver_addr) return resolver_addr;
    else return "tcp://localhost:22000";
}

void Node::announceNode()
{
    log(TRACE, "Announcing node '%s' to resolver...", name_);
    b0::resolver_msgs::Request rq0;
    b0::resolver_msgs::AnnounceNodeRequest &rq = *rq0.mutable_announce_node();
    getNodeID(*rq.mutable_node_id());
    rq.set_node_name(name_);
    s_send(resolv_socket_, rq0);

    b0::resolver_msgs::Response rsp0;
    log(TRACE, "Waiting for response from resolver...");
    s_recv(resolv_socket_, rsp0);
    const b0::resolver_msgs::AnnounceNodeResponse &rsp = rsp0.announce_node();

    if(name_ != rsp.node_name())
    {
        log(WARN, "Warning: resolver changed this node name to '%s'", rsp.node_name());
    }
    name_ = rsp.node_name();

    xpub_sock_addr_ = rsp.xpub_sock_addr();
    log(TRACE, "Proxy's XPUB socket address: %s", xpub_sock_addr_);

    xsub_sock_addr_ = rsp.xsub_sock_addr();
    log(TRACE, "Proxy's XSUB socket address: %s", xsub_sock_addr_);

    logger_.connect(xsub_sock_addr_);
}

void Node::heartbeatLoop()
{
    zmq::socket_t socket(context_, ZMQ_REQ);
    socket.connect(resolverAddress());
    while(true)
    {
        b0::resolver_msgs::Request rq0;
        b0::resolver_msgs::HeartBeatRequest &rq = *rq0.mutable_heartbeat();
        getNodeID(*rq.mutable_node_id());
        s_send(socket, rq0);

        b0::resolver_msgs::Response rsp0;
        s_recv(socket, rsp0);
        const b0::resolver_msgs::HeartBeatResponse &rsp = rsp0.heartbeat();
        if(!rsp.ok()) break;
        boost::this_thread::sleep_for(boost::chrono::seconds{1});
    }
}

void Node::spinOnce()
{
    // publishers don't need to spin

    // spin subscribers
    for(auto it = subscribers_.begin(); it != subscribers_.end(); ++it)
    {
        (*it)->spinOnce();
    }

    // service clients don't need to spin

    // spin service servers
    for(auto it = service_servers_.begin(); it != service_servers_.end(); ++it)
    {
        (*it)->spinOnce();
    }
}

void Node::spin(double spinRate)
{
    while(true)
    {
        spinOnce();
        boost::this_thread::sleep(boost::posix_time::microseconds(1000000. / spinRate));
    }
}

} // namespace b0


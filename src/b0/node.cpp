#include <b0/node.h>
#include <b0/publisher.h>
#include <b0/subscriber.h>
#include <b0/service_client.h>
#include <b0/service_server.h>
#include <b0/logger/logger.h>

#include <iostream>
#include <cstdlib>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "resolver.pb.h"

namespace b0
{

std::atomic<bool> Node::quit_flag_(false);

bool Node::sigint_handler_setup_ = false;

Node::Node(std::string nodeName)
    : context_(1),
      resolv_cli_(this, "resolv", false),
      name_(nodeName),
      state_(State::Created),
      thread_id_(boost::this_thread::get_id()),
      p_logger_(new logger::Logger(this)),
      shutdown_flag_(false)
{
    setupSIGINTHandler();

    // initialize time sync state variables:
    timesync_.target_offset_ = 0;
    timesync_.last_offset_time_ = hardwareTimeUSec();
    timesync_.last_offset_value_ = 0;
    timesync_.max_slope_ = 0.5;
}

Node::~Node()
{
    if(logger::Logger *p_logger = dynamic_cast<logger::Logger*>(p_logger_))
        delete p_logger;
}

void Node::init()
{
    if(state_ != State::Created)
        throw std::runtime_error("Cannot call init() in current state");

    log(DEBUG, "Initialization...");

    resolv_cli_.setRemoteAddress(resolverAddress());
    resolv_cli_.init(); // resolv_cli_ is not managed

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

    state_ = State::Ready;

    log(DEBUG, "Initialization complete.");
}

void Node::shutdown()
{
    if(state_ != State::Ready)
        throw std::runtime_error("Cannot call shutdown() in current state");

    log(DEBUG, "Shutting down...");

    shutdown_flag_ = true;

    log(DEBUG, "Shutting complete.");
}

bool Node::shutdownRequested() const
{
    return shutdown_flag_ || quit_flag_.load();
}

void Node::spinOnce()
{
    if(state_ != State::Ready)
        throw std::runtime_error("Cannot call spinOnce() in current state");

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
    if(state_ != State::Ready)
        throw std::runtime_error("Cannot call spin() in current state");

    while(!shutdownRequested())
    {
        spinOnce();
        boost::this_thread::sleep(boost::posix_time::microseconds(1000000. / spinRate));
    }

    log(INFO, "Node shutdown requested");

    cleanup();
}

void Node::cleanup()
{
    if(state_ != State::Ready)
        throw std::runtime_error("Cannot call cleanup() in current state");

    // stop the heartbeat_thread so that the last zmq socket will be destroyed
    // and we avoid an unclean exit (zmq::error_t: Context was terminated)
    heartbeat_thread_.interrupt();
    heartbeat_thread_.join();

    log(DEBUG, "Cleanup publishers...");
    for(auto it = publishers_.begin(); it != publishers_.end(); ++it)
        (*it)->cleanup();
    log(DEBUG, "Cleanup service clients...");
    for(auto it = service_clients_.begin(); it != service_clients_.end(); ++it)
        (*it)->cleanup();
    log(DEBUG, "Cleanup service servers...");
    for(auto it = service_servers_.begin(); it != service_servers_.end(); ++it)
        (*it)->cleanup();
    log(DEBUG, "Cleanup subscribers...");
    for(auto it = subscribers_.begin(); it != subscribers_.end(); ++it)
        (*it)->cleanup();

    // inform resolver that we are shutting down
    notifyShutdown();

    resolv_cli_.cleanup(); // resolv_cli_ is not managed

    state_ = State::Terminated;
}

void Node::log(LogLevel level, std::string message)
{
    p_logger_->log(level, message);
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

Node::State Node::getState() const
{
    return state_;
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
    if(state_ != State::Created)
        throw std::runtime_error("Cannot create a publisher with an already initialized node");

    publishers_.insert(pub);
}

void Node::removePublisher(AbstractPublisher *pub)
{
    publishers_.erase(pub);
}

void Node::addSubscriber(AbstractSubscriber *sub)
{
    if(state_ != State::Created)
        throw std::runtime_error("Cannot create a subscriber with an already initialized node");

    subscribers_.insert(sub);
}

void Node::removeSubscriber(AbstractSubscriber *sub)
{
    subscribers_.erase(sub);
}

void Node::addServiceClient(AbstractServiceClient *cli)
{
    if(state_ != State::Created)
        throw std::runtime_error("Cannot create a service client with an already initialized node");

    service_clients_.insert(cli);
}

void Node::removeServiceClient(AbstractServiceClient *cli)
{
    service_clients_.erase(cli);
}

void Node::addServiceServer(AbstractServiceServer *srv)
{
    if(state_ != State::Created)
        throw std::runtime_error("Cannot create a service server with an already initialized node");

    service_servers_.insert(srv);
}

void Node::removeServiceServer(AbstractServiceServer *srv)
{
    service_servers_.erase(srv);
}

std::string Node::hostname()
{
    const char *host_addr = std::getenv("BWF_HOST_ID");
    if(host_addr) return host_addr;
    else return boost::asio::ip::host_name();
}

int Node::pid()
{
    return ::getpid();
}

std::string Node::threadID()
{
    return boost::lexical_cast<std::string>(thread_id_);
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
    b0::resolver_msgs::NodeID &node_id = *rq.mutable_node_id();
    node_id.set_host_id(hostname());
    node_id.set_process_id(pid());
    node_id.set_thread_id(threadID());
    rq.set_node_name(name_);

    b0::resolver_msgs::Response rsp0;
    log(TRACE, "Waiting for response from resolver...");
    resolv_cli_.call(rq0, rsp0);
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

    if(logger::Logger *p_logger = dynamic_cast<logger::Logger*>(p_logger_))
        p_logger->connect(xsub_sock_addr_);
}

void Node::notifyShutdown()
{
    log(TRACE, "Notifying node shutdown to resolver...");
    b0::resolver_msgs::Request rq0;
    b0::resolver_msgs::ShutdownNodeRequest &rq = *rq0.mutable_shutdown_node();
    b0::resolver_msgs::NodeID &node_id = *rq.mutable_node_id();
    node_id.set_host_id(hostname());
    node_id.set_process_id(pid());
    node_id.set_thread_id(threadID());

    b0::resolver_msgs::Response rsp0;
    log(TRACE, "Waiting for response from resolver...");
    resolv_cli_.call(rq0, rsp0);
    const b0::resolver_msgs::ShutdownNodeResponse &rsp = rsp0.shutdown_node();

    if(!rsp.ok())
        log(WARN, "resolver has some problem with our shutdown... alas");
}

void Node::heartbeatLoop()
{
    ResolverServiceClient resolv_cli(this, "resolv", false);
    resolv_cli.setRemoteAddress(resolverAddress());
    resolv_cli.init();
    while(!shutdownRequested())
    {
        b0::resolver_msgs::Request rq0;
        b0::resolver_msgs::HeartBeatRequest &rq = *rq0.mutable_heartbeat();
        b0::resolver_msgs::NodeID &node_id = *rq.mutable_node_id();
        node_id.set_host_id(hostname());
        node_id.set_process_id(pid());
        node_id.set_thread_id(threadID());
        int64_t sendTime = hardwareTimeUSec();

        b0::resolver_msgs::Response rsp0;
        resolv_cli.call(rq0, rsp0);
        int64_t recvTime = hardwareTimeUSec();
        int64_t rtt = recvTime - sendTime;
        const b0::resolver_msgs::HeartBeatResponse &rsp = rsp0.heartbeat();
        if(!rsp.ok()) break;
        updateTime(rsp.time_usec() + rtt / 2);
        boost::this_thread::sleep_for(boost::chrono::seconds{1});
    }
    resolv_cli.cleanup();

    log(INFO, "Heartbeat thread terminating.");
}

int64_t Node::hardwareTimeUSec() const
{
    static boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
    boost::posix_time::ptime t = boost::posix_time::microsec_clock::local_time();
    return (t - epoch).total_microseconds();
}

int64_t Node::timeUSec()
{
    return hardwareTimeUSec() + constantRateAdjustedOffset();
}

int64_t Node::constantRateAdjustedOffset()
{
    boost::mutex::scoped_lock lock(timesync_.mutex_);

    int64_t offset_delta = timesync_.target_offset_ - timesync_.last_offset_value_;
    int64_t slope_time = abs(offset_delta) / timesync_.max_slope_;
    int64_t t = hardwareTimeUSec() - timesync_.last_offset_time_;
    if(t > slope_time)
        return timesync_.target_offset_;
    else
        return timesync_.last_offset_value_ + offset_delta * t / slope_time;
}

void Node::updateTime(int64_t remoteTime)
{
    int64_t last_offset_value = constantRateAdjustedOffset();
    int64_t local_time = hardwareTimeUSec();

    {
        boost::mutex::scoped_lock lock(timesync_.mutex_);

        timesync_.last_offset_value_ = last_offset_value;
        timesync_.last_offset_time_ = local_time;
        timesync_.target_offset_ = remoteTime - local_time;
    }
}

void Node::signalHandler(int s)
{
    quit_flag_.store(true);
}

void Node::setupSIGINTHandler()
{
    if(sigint_handler_setup_) return;

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &Node::signalHandler;
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    sigint_handler_setup_ = true;
}

} // namespace b0


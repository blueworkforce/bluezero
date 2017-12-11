#include <b0/node.h>
#include <b0/publisher.h>
#include <b0/subscriber.h>
#include <b0/service_client.h>
#include <b0/service_server.h>
#include <b0/exceptions.h>
#include <b0/logger/logger.h>
#include <b0/utils/thread_name.h>

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
      resolv_cli_(this),
      name_(nodeName),
      state_(State::Created),
      thread_id_(boost::this_thread::get_id()),
      p_logger_(new logger::Logger(this)),
      shutdown_flag_(false)
{
    set_thread_name("main");

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
        throw exception::InvalidStateTransition("Cannot call init() in current state");

    log(DEBUG, "Initialization...");

    resolv_cli_.init(); // resolv_cli_ is not managed

    announceNode();

    startHeartbeatThread();

    log(DEBUG, "Initializing sockets...");
    for(auto socket : sockets_)
        socket->init();

    state_ = State::Ready;

    log(DEBUG, "Initialization complete.");
}

void Node::shutdown()
{
    if(state_ != State::Ready)
        throw exception::InvalidStateTransition("Cannot call shutdown() in current state");

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
        throw exception::InvalidStateTransition("Cannot call spinOnce() in current state");

    // spin sockets:
    for(auto socket : sockets_)
        socket->spinOnce();
}

void Node::spin(double spinRate)
{
    if(state_ != State::Ready)
        throw exception::InvalidStateTransition("Cannot call spin() in current state");

    log(INFO, "Node spinning...");

    while(!shutdownRequested())
    {
        spinOnce();
        boost::this_thread::sleep(boost::posix_time::microseconds(1000000. / spinRate));
    }

    log(INFO, "Node cleanup...");

    cleanup();

    log(INFO, "spin() finished");
}

void Node::cleanup()
{
    if(state_ != State::Ready)
        throw exception::InvalidStateTransition("Cannot call cleanup() in current state");

    // stop the heartbeat_thread so that the last zmq socket will be destroyed
    // and we avoid an unclean exit (zmq::error_t: Context was terminated)
    log(DEBUG, "Killing heartbeat thread...");
    heartbeat_thread_.interrupt();
    heartbeat_thread_.join();

    log(DEBUG, "Cleanup sockets...");
    for(auto socket : sockets_)
        socket->cleanup();

    // inform resolver that we are shutting down
    notifyShutdown();

    resolv_cli_.cleanup(); // resolv_cli_ is not managed

    state_ = State::Terminated;
}

void Node::log(LogLevel level, std::string message) const
{
    if(boost::this_thread::get_id() != thread_id_)
        throw exception::Exception("cannot call Node::log() from another thread");

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

void Node::addSocket(socket::Socket *socket)
{
    if(state_ != State::Created)
        throw exception::Exception("Cannot create a socket with an already initialized node");

    sockets_.insert(socket);
}

void Node::removeSocket(socket::Socket *socket)
{
    sockets_.erase(socket);
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
    boost::format fmt("tcp://%s:%d");
    return (fmt % hostname() % freeTCPPort()).str();
}

void Node::announceNode()
{
    resolv_cli_.announceNode(name_, xpub_sock_addr_, xsub_sock_addr_);

    if(logger::Logger *p_logger = dynamic_cast<logger::Logger*>(p_logger_))
        p_logger->connect(xsub_sock_addr_);
}

void Node::notifyShutdown()
{
    resolv_cli_.notifyShutdown();
}

void Node::heartbeatLoop()
{
    set_thread_name("HB");

    resolver::Client resolv_cli(this);
    resolv_cli.init();

    while(!shutdownRequested())
    {
        int64_t time_usec;
        resolv_cli.sendHeartbeat(&time_usec);
        updateTime(time_usec);
        boost::this_thread::sleep_for(boost::chrono::seconds{1});
    }

    resolv_cli.cleanup();
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

#ifndef WIN32
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &Node::signalHandler;
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
#endif

    sigint_handler_setup_ = true;
}

} // namespace b0


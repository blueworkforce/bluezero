#include <b0/service_server.h>
#include <b0/node.h>

#include "resolver.pb.h"
#include "logger.pb.h"

#include <zmq.hpp>

namespace b0
{

ServiceServer::ServiceServer(Node *node, std::string service_name, boost::function<void(const std::string&, std::string&)> callback, bool managed, bool notify_graph)
    : Socket(node, ZMQ_REP, service_name, managed),
      notify_graph_(notify_graph),
      bind_addr_(""),
      callback_(callback)
{
}

ServiceServer::~ServiceServer()
{
}

void ServiceServer::log(LogLevel level, std::string message) const
{
    boost::format fmt("ServiceServer(%s): %s");
    Socket::log(level, (fmt % name_ % message).str());
}

void ServiceServer::init()
{
    bind();
    announce();

    if(notify_graph_)
        node_.notifyService(name_, false, true);
}

void ServiceServer::cleanup()
{
    unbind();

    if(notify_graph_)
        node_.notifyService(name_, false, false);
}

void ServiceServer::spinOnce()
{
    if(callback_.empty()) return;

    while(poll())
    {
        std::string req, rep;
        readRaw(req);
        callback_(req, rep);
        writeRaw(rep);
    }
}

std::string ServiceServer::getServiceName()
{
    return name_;
}

void ServiceServer::bind()
{
    boost::format fmt("tcp://%s:%d");
    std::string host = node_.hostname();
    int port = node_.freeTCPPort();
    bind_addr_ = (fmt % "*" % port).str();
    remote_addr_ = (fmt % host % port).str();
    Socket::bind(bind_addr_);
    log(debug, "Bound to %s", bind_addr_);
}

void ServiceServer::unbind()
{
    //Socket::unbind(bind_addr_); // FIXME: causes a zmq error on node shutdown
}

void ServiceServer::announce()
{
    log(trace, "Announcing %s to resolver...", remote_addr_);
    node_.announceService(name_, remote_addr_);
}

void ServiceServer::bind(std::string address)
{
    Socket::bind(address);
    log(debug, "Bound to additional address %s", address);
}

} // namespace b0


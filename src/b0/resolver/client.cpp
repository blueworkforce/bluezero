#include <b0/resolver/client.h>
#include <b0/node.h>
#include <b0/messages.h>
#include <b0/exceptions.h>
#include <b0/utils/env.h>

#include <zmq.hpp>

namespace b0
{

namespace resolver
{

Client::Client(b0::Node *node)
    : ServiceClient(node, "resolv", false, false),
      announce_timeout_(-1)
{
    if(!node)
        throw exception::Exception("node cannot be null");

    std::string resolver_addr = b0::env::get("BWF_RESOLVER");
    if(resolver_addr != "")
    {
        log(warn, "BWF_RESOLVER variable is deprecated. Use B0_RESOLVER instead.");
    }
    else
    {
        resolver_addr = b0::env::get("B0_RESOLVER");
    }
    setRemoteAddress(resolver_addr != "" ? resolver_addr : "tcp://localhost:22000");
}

Client::~Client()
{
}

void Client::setAnnounceTimeout(int timeout)
{
    announce_timeout_ = timeout;
}

void Client::announceNode(std::string &node_name, std::string &xpub_sock_addr, std::string &xsub_sock_addr)
{
    int old_timeout = getReadTimeout();
    setReadTimeout(announce_timeout_);

    log(trace, "Announcing node '%s' to resolver...", node_name);
    b0::message::AnnounceNodeRequest rq;
    rq.node_name = node_name;

    b0::message::AnnounceNodeResponse rsp;
    log(trace, "Waiting for response from resolver...");
    call(rq, rsp);

    setReadTimeout(old_timeout);

    if(!rsp.ok)
        throw exception::Exception("announceNode failed");

    if(node_name != rsp.node_name)
    {
        log(warn, "Warning: resolver changed this node name to '%s'", rsp.node_name);
    }
    node_name = rsp.node_name;

    xpub_sock_addr = rsp.xpub_sock_addr;
    log(trace, "Proxy's XPUB socket address: %s", xpub_sock_addr);

    xsub_sock_addr = rsp.xsub_sock_addr;
    log(trace, "Proxy's XSUB socket address: %s", xsub_sock_addr);
}

void Client::notifyShutdown()
{
    b0::message::ShutdownNodeRequest rq;
    rq.node_name = node_.getName();

    b0::message::ShutdownNodeResponse rsp;
    call(rq, rsp);

    if(!rsp.ok)
        throw exception::Exception("notifyShutdown failed");
}

void Client::sendHeartbeat(int64_t *time_usec)
{
    b0::message::HeartbeatRequest rq;
    rq.node_name = node_.getName();
    int64_t sendTime = node_.hardwareTimeUSec();

    b0::message::HeartbeatResponse rsp;
    call(rq, rsp);

    if(!rsp.ok)
        throw exception::Exception("sendHeartbeat failed");

    if(time_usec)
    {
        int64_t recvTime = node_.hardwareTimeUSec();
        int64_t rtt = recvTime - sendTime;
        *time_usec = rsp.time_usec + rtt / 2;
    }
}

void Client::notifyTopic(std::string topic_name, bool reverse, bool active)
{
    b0::message::NodeTopicRequest rq;
    rq.node_name = node_.getName();
    rq.topic_name = topic_name;
    rq.reverse = reverse;
    rq.active = active;

    b0::message::NodeTopicResponse rsp;
    call(rq, rsp);
}

void Client::notifyService(std::string service_name, bool reverse, bool active)
{
    b0::message::NodeServiceRequest rq;
    rq.node_name = node_.getName();
    rq.service_name = service_name;
    rq.reverse = reverse;
    rq.active = active;

    b0::message::NodeServiceResponse rsp;
    call(rq, rsp);
}

void Client::announceService(std::string name, std::string addr)
{
    b0::message::AnnounceServiceRequest rq;
    rq.node_name = node_.getName();
    rq.service_name = name;
    rq.sock_addr = addr;

    b0::message::AnnounceServiceResponse rsp;
    call(rq, rsp);

    if(!rsp.ok)
        throw exception::Exception("announceService failed");
}

void Client::resolveService(std::string name, std::string &addr)
{
    b0::message::ResolveServiceRequest rq;
    rq.service_name = name;

    b0::message::ResolveServiceResponse rsp;
    call(rq, rsp);

    if(!rsp.ok)
        throw exception::NameResolutionError(name);

    addr = rsp.sock_addr;
}

void Client::getGraph(b0::message::Graph &graph)
{
    b0::message::GetGraphRequest rq;
    b0::message::GetGraphResponse rsp;
    call(rq, rsp);
    graph = rsp.graph;
}

} // namespace resolver

} // namespace b0


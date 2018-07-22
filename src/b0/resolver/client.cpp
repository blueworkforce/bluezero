#include <b0/resolver/client.h>
#include <b0/node.h>
#include <b0/message/resolv_request.h>
#include <b0/message/resolv_response.h>
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
    b0::message::ResolvRequest rq0;
    rq0.announce_node.emplace();
    b0::message::AnnounceNodeRequest &rq = *rq0.announce_node;
    rq.node_name = node_name;

    b0::message::ResolvResponse rsp0;
    rsp0.announce_node.emplace();
    b0::message::AnnounceNodeResponse &rsp = *rsp0.announce_node;
    log(trace, "Waiting for response from resolver...");
    call(rq0, rsp0);

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
    b0::message::ResolvRequest rq0;
    rq0.shutdown_node.emplace();
    b0::message::ShutdownNodeRequest &rq = *rq0.shutdown_node;
    rq.node_name = node_.getName();

    b0::message::ResolvResponse rsp0;
    rsp0.shutdown_node.emplace();
    b0::message::ShutdownNodeResponse &rsp = *rsp0.shutdown_node;
    call(rq0, rsp0);

    if(!rsp.ok)
        throw exception::Exception("notifyShutdown failed");
}

void Client::sendHeartbeat(int64_t *time_usec)
{
    b0::message::ResolvRequest rq0;
    rq0.heartbeat.emplace();
    b0::message::HeartbeatRequest &rq = *rq0.heartbeat;
    rq.node_name = node_.getName();
    int64_t sendTime = node_.hardwareTimeUSec();

    b0::message::ResolvResponse rsp0;
    rsp0.heartbeat.emplace();
    b0::message::HeartbeatResponse &rsp = *rsp0.heartbeat;
    call(rq0, rsp0);

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
    b0::message::ResolvRequest rq0;
    rq0.node_topic.emplace();
    b0::message::NodeTopicRequest &rq = *rq0.node_topic;
    rq.node_name = node_.getName();
    rq.topic_name = topic_name;
    rq.reverse = reverse;
    rq.active = active;

    b0::message::ResolvResponse rsp0;
    rsp0.node_topic.emplace();
    b0::message::NodeTopicResponse &rsp = *rsp0.node_topic;
    call(rq0, rsp0);
}

void Client::notifyService(std::string service_name, bool reverse, bool active)
{
    b0::message::ResolvRequest rq0;
    rq0.node_service.emplace();
    b0::message::NodeServiceRequest &rq = *rq0.node_service;
    rq.node_name = node_.getName();
    rq.service_name = service_name;
    rq.reverse = reverse;
    rq.active = active;

    b0::message::ResolvResponse rsp0;
    rsp0.node_service.emplace();
    b0::message::NodeServiceResponse &rsp = *rsp0.node_service;
    call(rq0, rsp0);
}

void Client::announceService(std::string name, std::string addr)
{
    b0::message::ResolvRequest rq0;
    rq0.announce_service.emplace();
    b0::message::AnnounceServiceRequest &rq = *rq0.announce_service;
    rq.node_name = node_.getName();
    rq.service_name = name;
    rq.sock_addr = addr;

    b0::message::ResolvResponse rsp0;
    rsp0.announce_service.emplace();
    b0::message::AnnounceServiceResponse &rsp = *rsp0.announce_service;
    call(rq0, rsp0);

    if(!rsp.ok)
        throw exception::Exception("announceService failed");
}

void Client::resolveService(std::string name, std::string &addr)
{
    b0::message::ResolvRequest rq0;
    rq0.resolve_service.emplace();
    b0::message::ResolveServiceRequest &rq = *rq0.resolve_service;
    rq.service_name = name;

    b0::message::ResolvResponse rsp0;
    rsp0.resolve_service.emplace();
    b0::message::ResolveServiceResponse &rsp = *rsp0.resolve_service;
    call(rq0, rsp0);

    if(!rsp.ok)
        throw exception::NameResolutionError(name);

    addr = rsp.sock_addr;
}

void Client::getGraph(b0::message::Graph &graph)
{
    b0::message::ResolvRequest rq0;
    rq0.get_graph.emplace();
    b0::message::GetGraphRequest &rq = *rq0.get_graph;
    b0::message::ResolvResponse rsp0;
    rsp0.get_graph.emplace();
    b0::message::GetGraphResponse &rsp = *rsp0.get_graph;
    call(rq0, rsp0);
    graph = rsp.graph;
}

} // namespace resolver

} // namespace b0


#include <b0/resolver/client.h>
#include <b0/node.h>
#include <b0/exceptions.h>

#include "resolver.pb.h"

namespace b0
{

namespace resolver
{

Client::Client(b0::Node *node)
    : ServiceClient<b0::resolver_msgs::Request, b0::resolver_msgs::Response>(node, "resolv", false, false)
{
    if(!node)
        throw exception::Exception("node cannot be null");

    const char *resolver_addr = std::getenv("BWF_RESOLVER");
    setRemoteAddress(resolver_addr ? resolver_addr : "tcp://localhost:22000");
}

Client::~Client()
{
}

void Client::announceNode(std::string &node_name, std::string &xpub_sock_addr, std::string &xsub_sock_addr)
{
    log(trace, "Announcing node '%s' to resolver...", node_name);
    b0::resolver_msgs::Request rq0;
    b0::resolver_msgs::AnnounceNodeRequest &rq = *rq0.mutable_announce_node();
    b0::resolver_msgs::NodeID &node_id = *rq.mutable_node_id();
    node_id.set_host_id(node_.hostname());
    node_id.set_process_id(node_.pid());
    node_id.set_thread_id(node_.threadID());
    rq.set_node_name(node_name);

    b0::resolver_msgs::Response rsp0;
    log(trace, "Waiting for response from resolver...");
    call(rq0, rsp0);
    const b0::resolver_msgs::AnnounceNodeResponse &rsp = rsp0.announce_node();

    if(!rsp.ok())
        throw exception::Exception("announceNode failed");

    if(node_name != rsp.node_name())
    {
        log(warn, "Warning: resolver changed this node name to '%s'", rsp.node_name());
    }
    node_name = rsp.node_name();

    xpub_sock_addr = rsp.xpub_sock_addr();
    log(trace, "Proxy's XPUB socket address: %s", xpub_sock_addr);

    xsub_sock_addr = rsp.xsub_sock_addr();
    log(trace, "Proxy's XSUB socket address: %s", xsub_sock_addr);
}

void Client::notifyShutdown()
{
    b0::resolver_msgs::Request rq0;
    b0::resolver_msgs::ShutdownNodeRequest &rq = *rq0.mutable_shutdown_node();
    b0::resolver_msgs::NodeID &node_id = *rq.mutable_node_id();
    node_id.set_host_id(node_.hostname());
    node_id.set_process_id(node_.pid());
    node_id.set_thread_id(node_.threadID());

    b0::resolver_msgs::Response rsp0;
    call(rq0, rsp0);
    const b0::resolver_msgs::ShutdownNodeResponse &rsp = rsp0.shutdown_node();

    if(!rsp.ok())
        throw exception::Exception("notifyShutdown failed");
}

void Client::sendHeartbeat(int64_t *time_usec, std::string host_id, int process_id, std::string thread_id)
{
    if(host_id == "")
        host_id = node_.hostname();

    if(process_id == 0)
        process_id = node_.pid();

    if(thread_id == "")
        thread_id = node_.threadID();

    b0::resolver_msgs::Request rq0;
    b0::resolver_msgs::HeartBeatRequest &rq = *rq0.mutable_heartbeat();
    b0::resolver_msgs::NodeID &node_id = *rq.mutable_node_id();
    node_id.set_host_id(host_id);
    node_id.set_process_id(process_id);
    node_id.set_thread_id(thread_id);
    int64_t sendTime = node_.hardwareTimeUSec();

    b0::resolver_msgs::Response rsp0;
    call(rq0, rsp0);
    const b0::resolver_msgs::HeartBeatResponse &rsp = rsp0.heartbeat();

    if(!rsp.ok())
        throw exception::Exception("sendHeartbeat failed");

    if(time_usec)
    {
        int64_t recvTime = node_.hardwareTimeUSec();
        int64_t rtt = recvTime - sendTime;
        *time_usec = rsp.time_usec() + rtt / 2;
    }
}

void Client::notifyTopic(std::string topic_name, bool reverse, bool active)
{
    b0::resolver_msgs::Request rq0;
    b0::resolver_msgs::NodeTopicRequest &req = *rq0.mutable_node_topic();
    req.set_node_name(node_.getName());
    req.set_topic_name(topic_name);
    req.set_reverse(reverse);
    req.set_active(active);

    b0::resolver_msgs::Response rsp0;
    call(rq0, rsp0);
}

void Client::notifyService(std::string service_name, bool reverse, bool active)
{
    b0::resolver_msgs::Request rq0;
    b0::resolver_msgs::NodeServiceRequest &req = *rq0.mutable_node_service();
    req.set_node_name(node_.getName());
    req.set_service_name(service_name);
    req.set_reverse(reverse);
    req.set_active(active);

    b0::resolver_msgs::Response rsp0;
    call(rq0, rsp0);
}

void Client::announceService(std::string name, std::string addr)
{
    b0::resolver_msgs::Request rq0;
    b0::resolver_msgs::AnnounceServiceRequest &rq = *rq0.mutable_announce_service();
    b0::resolver_msgs::NodeID &node_id = *rq.mutable_node_id();
    node_id.set_host_id(node_.hostname());
    node_id.set_process_id(node_.pid());
    node_id.set_thread_id(node_.threadID());
    rq.set_service_name(name);
    rq.set_sock_addr(addr);

    b0::resolver_msgs::Response rsp0;
    call(rq0, rsp0);
    const b0::resolver_msgs::AnnounceServiceResponse &rsp = rsp0.announce_service();

    if(!rsp.ok())
        throw exception::Exception("announceService failed");
}

void Client::resolveService(std::string name, std::string &addr)
{
    b0::resolver_msgs::Request rq0;
    b0::resolver_msgs::ResolveServiceRequest &rq = *rq0.mutable_resolve();
    rq.set_service_name(name);

    b0::resolver_msgs::Response rsp0;
    call(rq0, rsp0);
    const b0::resolver_msgs::ResolveServiceResponse &rsp = rsp0.resolve();

    if(!rsp.ok())
        throw exception::Exception("resolveService failed");

    addr = rsp.sock_addr();
}

void Client::getGraph(b0::resolver_msgs::Graph &graph)
{
    b0::resolver_msgs::Request req;
    b0::resolver_msgs::GetGraphRequest &gg = *req.mutable_get_graph();
    b0::resolver_msgs::Response resp;
    call(req, resp);
    graph = resp.get_graph().graph();
}

} // namespace resolver

} // namespace b0


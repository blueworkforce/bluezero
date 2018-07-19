#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <b0/resolver/resolver.h>
#include <b0/resolver/client.h>
#include <b0/logger/logger.h>
#include <b0/utils/env.h>
#include <b0/utils/thread_name.h>

#include <zmq.hpp>

namespace b0
{

namespace resolver
{

ResolverServiceServer::ResolverServiceServer(Resolver *resolver)
    : ServiceServer(resolver, "resolv", &Resolver::handle, resolver, true, false),
      resolver_(resolver)
{
}

void ResolverServiceServer::announce()
{
    b0::message::AnnounceServiceRequest rq;
    rq.node_name = node_.getName();
    rq.service_name = name_;
    rq.sock_addr = remote_addr_;
    b0::message::AnnounceServiceResponse rsp;
    resolver_->handleAnnounceService(rq, rsp);
    resolver_->onNodeServiceOfferStart(resolver_->getName(), name_);
}

Resolver::Resolver()
    : Node("resolver"),
      resolv_server_(this),
      graph_pub_(this, "graph", true, false)
{
}

Resolver::~Resolver()
{
    pub_proxy_thread_.interrupt();
    //pub_proxy_thread_.join(); // FIXME: this makes the process hang on quit
}

void Resolver::init()
{
    resolv_addr_ = "inproc://resolv";

    Node::init();

    resolv_server_.bind((boost::format("tcp://*:%s") % resolverPort()).str());
    resolv_server_.bind("inproc://resolv"); // for socket to self

    // setup XPUB-XSUB proxy addresses
    // those will be sent to nodes in response to announce
    int xsub_proxy_port_ = freeTCPPort();
    xsub_proxy_addr_ = address(hostAddress(), xsub_proxy_port_);
    log(trace, "XSUB address is %s", xsub_proxy_addr_);
    int xpub_proxy_port_ = freeTCPPort();
    xpub_proxy_addr_ = address(hostAddress(), xpub_proxy_port_);
    log(trace, "XPUB address is %s", xpub_proxy_addr_);
    // run XPUB-XSUB proxy:
    pub_proxy_thread_ = boost::thread(&Resolver::pubProxy, this, xsub_proxy_port_, xpub_proxy_port_);

    // run heartbeat sweeper (to detect when nodes go offline):
    heartbeat_sweeper_thread_ = boost::thread(&Resolver::heartbeatSweeper, this);

    // we have to manually call this because graph_pub_ doesn't send graph notify:
    // (has to be disabled because resolver is a special kind of node)
    onNodeTopicPublishStart(getName(), graph_pub_.getTopicName());

    log(info, "Ready.");
}

void Resolver::shutdown()
{
    Node::shutdown();

    pub_proxy_thread_.interrupt();
    pub_proxy_thread_.join();
}

std::string Resolver::getXPUBSocketAddress() const
{
    return "inproc://xpub_proxy";
}

std::string Resolver::getXSUBSocketAddress() const
{
    return "inproc://xsub_proxy";
}

void Resolver::announceNode()
{
    // directly route this call to the handler, otherwise it will cause a deadlock
    b0::message::AnnounceNodeRequest rq;
    rq.node_name = getName();
    b0::message::AnnounceNodeResponse rsp;
    handleAnnounceNode(rq, rsp);

    if(logger::Logger *p_logger = dynamic_cast<logger::Logger*>(p_logger_))
        p_logger->connect("inproc://xsub_proxy");
}

void Resolver::notifyShutdown()
{
    // directly route this call to the handler, otherwise it will cause a deadlock
#if 0
    b0::message::ShutdownNodeRequest rq;
    rq.node_name = getName();
    b0::message::ShutdownNodeResponse rsp;
    handleShutdownNode(rq, rsp);
#else
    // nothing to do really
#endif
}

void Resolver::onNodeConnected(std::string name)
{
}

void Resolver::onNodeDisconnected(std::string name)
{
    resolver::NodeEntry *e = nodeByName(name);

    for(resolver::ServiceEntry *s : e->services)
        services_by_name_.erase(s->name);
    nodes_by_name_.erase(name);

    std::set<std::pair<std::string, std::string> > npt, nst, nos, nus;

    for(auto x : node_publishes_topic_)
        if(x.first == name)
            npt.insert(x);

    for(auto x : node_subscribes_topic_)
        if(x.first == name)
            nst.insert(x);

    for(auto x : node_offers_service_)
        if(x.first == name)
            nos.insert(x);

    for(auto x : node_uses_service_)
        if(x.first == name)
            nus.insert(x);

    for(auto x : npt) onNodeTopicPublishStop(x.first, x.second);
    for(auto x : nst) onNodeTopicSubscribeStop(x.first, x.second);
    for(auto x : nos) onNodeServiceOfferStop(x.first, x.second);
    for(auto x : nus) onNodeServiceUseStop(x.first, x.second);

    onGraphChanged();
}

void Resolver::onNodeTopicPublishStart(std::string node_name, std::string topic_name)
{
    log(info, "Graph: node '%s' publishes on topic '%s'", node_name, topic_name);
    node_publishes_topic_.insert(std::make_pair(node_name, topic_name));
}

void Resolver::onNodeTopicPublishStop(std::string node_name, std::string topic_name)
{
    log(info, "Graph: node '%s' stops publishing on topic '%s'", node_name, topic_name);
    node_publishes_topic_.erase(std::make_pair(node_name, topic_name));
}

void Resolver::onNodeTopicSubscribeStart(std::string node_name, std::string topic_name)
{
    log(info, "Graph: node '%s' subscribes to topic '%s'", node_name, topic_name);
    node_subscribes_topic_.insert(std::make_pair(node_name, topic_name));
}

void Resolver::onNodeTopicSubscribeStop(std::string node_name, std::string topic_name)
{
    log(info, "Graph: node '%s' stops subscribing to topic '%s'", node_name, topic_name);
    node_subscribes_topic_.erase(std::make_pair(node_name, topic_name));
}

void Resolver::onNodeServiceOfferStart(std::string node_name, std::string service_name)
{
    log(info, "Graph: node '%s' offers service '%s'", node_name, service_name);
    node_offers_service_.insert(std::make_pair(node_name, service_name));
}

void Resolver::onNodeServiceOfferStop(std::string node_name, std::string service_name)
{
    log(info, "Graph: node '%s' stops offering service '%s'", node_name, service_name);
    node_offers_service_.erase(std::make_pair(node_name, service_name));
}

void Resolver::onNodeServiceUseStart(std::string node_name, std::string service_name)
{
    log(info, "Graph: node '%s' connects to service '%s'", node_name, service_name);
    node_uses_service_.insert(std::make_pair(node_name, service_name));
}

void Resolver::onNodeServiceUseStop(std::string node_name, std::string service_name)
{
    log(info, "Graph: node '%s' disconnects from service '%s'", node_name, service_name);
    node_uses_service_.erase(std::make_pair(node_name, service_name));
}

void Resolver::pubProxy(int xsub_proxy_port, int xpub_proxy_port)
{
    set_thread_name("XPROXY");

    zmq::context_t &context_ = *reinterpret_cast<zmq::context_t*>(getContext());

    zmq::socket_t proxy_in_sock_(context_, ZMQ_XSUB);
    std::string xsub_proxy_addr = address(xsub_proxy_port);
    proxy_in_sock_.bind(xsub_proxy_addr);
    proxy_in_sock_.bind("inproc://xsub_proxy");

    zmq::socket_t proxy_out_sock_(context_, ZMQ_XPUB);
    std::string xpub_proxy_addr = address(xpub_proxy_port);
    proxy_out_sock_.bind(xpub_proxy_addr);
    proxy_out_sock_.bind("inproc://xpub_proxy");

    try
    {
#ifdef __GNUC__
        zmq::proxy(static_cast<void*>(proxy_in_sock_), static_cast<void*>(proxy_out_sock_), nullptr);
#else
        zmq::proxy(proxy_in_sock_, proxy_out_sock_, nullptr);
#endif
    }
    catch(zmq::error_t &ex)
    {
    }
}

bool Resolver::nodeNameExists(std::string name)
{
    return name == "node" || nodes_by_name_.find(name) != nodes_by_name_.end();
}

std::string Resolver::hostAddress()
{
    std::string host_addr = b0::env::get("BWF_HOST_ID");
    if(host_addr != "")
    {
        log(warn, "BWF_HOST_ID varable is deprecated. Use B0_HOST_ID instead.");
        return host_addr;
    }
    host_addr = b0::env::get("B0_HOST_ID");
    if(host_addr != "")
    {
        return host_addr;
    }
    return boost::asio::ip::host_name();
}

int Resolver::resolverPort() const
{
    int resolver_port = b0::env::getInt("BWF_RESOLVER_PORT");
    if(resolver_port)
    {
        log(warn, "BWF_RESOLVER_PORT variable is deprecated. Use B0_RESOLVER_PORT instead.");
        return resolver_port;
    }
    resolver_port = b0::env::getInt("B0_RESOLVER_PORT");
    if(resolver_port)
    {
        return resolver_port;
    }
    return 22000;
}

std::string Resolver::address(std::string host, int port)
{
    boost::format f("tcp://%s:%d");
    return (f % host % port).str();
}

std::string Resolver::address(int port)
{
    return address("*", port);
}

resolver::NodeEntry * Resolver::nodeByName(std::string node_name)
{
    auto it = nodes_by_name_.find(node_name);
    return it == nodes_by_name_.end() ? 0 : it->second;
}

resolver::ServiceEntry * Resolver::serviceByName(std::string service_name)
{
    auto it = services_by_name_.find(service_name);
    return it == services_by_name_.end() ? 0 : it->second;
}

void Resolver::heartbeat(resolver::NodeEntry *node_entry)
{
    node_entry->last_heartbeat = boost::posix_time::second_clock::local_time();
}

void Resolver::handle(const std::string &req, const std::string &reqtype, std::string &rep, std::string &reptype)
{
    if(reqtype == "AnnounceNodeRequest")
    {
        b0::message::AnnounceNodeRequest rq;
        rq.parseFromString(req);
        b0::message::AnnounceNodeResponse rsp;
        handleAnnounceNode(rq, rsp);
        rsp.serializeToString(rep);
        reptype = rsp.type();
    }
    else if(reqtype == "ShutdownNodeRequest")
    {
        b0::message::ShutdownNodeRequest rq;
        rq.parseFromString(req);
        b0::message::ShutdownNodeResponse rsp;
        handleShutdownNode(rq, rsp);
        rsp.serializeToString(rep);
        reptype = rsp.type();
    }
    else if(reqtype == "AnnounceServiceRequest")
    {
        b0::message::AnnounceServiceRequest rq;
        rq.parseFromString(req);
        b0::message::AnnounceServiceResponse rsp;
        handleAnnounceService(rq, rsp);
        rsp.serializeToString(rep);
        reptype = rsp.type();
    }
    else if(reqtype == "ResolveServiceRequest")
    {
        b0::message::ResolveServiceRequest rq;
        rq.parseFromString(req);
        b0::message::ResolveServiceResponse rsp;
        handleResolveService(rq, rsp);
        rsp.serializeToString(rep);
        reptype = rsp.type();
    }
    else if(reqtype == "HeartbeatRequest")
    {
        b0::message::HeartbeatRequest rq;
        rq.parseFromString(req);
        b0::message::HeartbeatResponse rsp;
        handleHeartbeat(rq, rsp);
        rsp.serializeToString(rep);
        reptype = rsp.type();
    }
    else if(reqtype == "NodeTopicRequest")
    {
        b0::message::NodeTopicRequest rq;
        rq.parseFromString(req);
        b0::message::NodeTopicResponse rsp;
        handleNodeTopic(rq, rsp);
        rsp.serializeToString(rep);
        reptype = rsp.type();
    }
    else if(reqtype == "NodeServiceRequest")
    {
        b0::message::NodeServiceRequest rq;
        rq.parseFromString(req);
        b0::message::NodeServiceResponse rsp;
        handleNodeService(rq, rsp);
        rsp.serializeToString(rep);
        reptype = rsp.type();
    }
    else if(reqtype == "GetGraphRequest")
    {
        b0::message::GetGraphRequest rq;
        rq.parseFromString(req);
        b0::message::GetGraphResponse rsp;
        handleGetGraph(rq, rsp);
        rsp.serializeToString(rep);
        reptype = rsp.type();
    }
    else
    {
        log(error, "received an unrecognized request type: %s", reqtype);
    }
}

std::string Resolver::makeUniqueNodeName(std::string nodeName)
{
    if(nodeName == "") nodeName = "node";
    std::string uniqueNodeName = nodeName;
    for(int i = 1; true; i++)
    {
        if(!nodeNameExists(uniqueNodeName)) break;
        uniqueNodeName = (boost::format("%s-%d") % nodeName % i).str();
    }
    return uniqueNodeName;
}

void Resolver::handleAnnounceNode(const b0::message::AnnounceNodeRequest &rq, b0::message::AnnounceNodeResponse &rsp)
{
    log(trace, "Received a AnnounceNodeRequest");
    std::string nodeName = makeUniqueNodeName(rq.node_name);
    resolver::NodeEntry *e = new resolver::NodeEntry;
    e->name = nodeName;
    heartbeat(e);
    nodes_by_name_[nodeName] = e;
    onNodeConnected(nodeName);
    onGraphChanged();
    rsp.node_name = e->name;
    rsp.xsub_sock_addr = xsub_proxy_addr_;
    rsp.xpub_sock_addr = xpub_proxy_addr_;
    rsp.ok = true;
    log(info, "New node has joined: '%s'", e->name);
}

void Resolver::handleShutdownNode(const b0::message::ShutdownNodeRequest &rq, b0::message::ShutdownNodeResponse &rsp)
{
    resolver::NodeEntry *ne = nodeByName(rq.node_name);
    if(!ne)
    {
        rsp.ok = false;
        log(error, "Invalid node name: %s", rq.node_name);
        return;
    }
    std::string node_name = ne->name;
    onNodeDisconnected(node_name);
    delete ne;
    rsp.ok = true;
    log(info, "Node '%s' has left", node_name);
}

void Resolver::handleAnnounceService(const b0::message::AnnounceServiceRequest &rq, b0::message::AnnounceServiceResponse &rsp)
{
    resolver::NodeEntry *ne = nodeByName(rq.node_name);
    if(!ne)
    {
        rsp.ok = false;
        log(error, "Invalid node name: %s", rq.node_name);
        return;
    }
    if(serviceByName(rq.service_name))
    {
        rsp.ok = false;
        log(error, "A service with name '%s' already exists", rq.service_name);
        return;
    }
    resolver::ServiceEntry *se = new resolver::ServiceEntry;
    se->node = ne;
    se->name = rq.service_name;
    se->addr = rq.sock_addr;
    services_by_name_[se->name] = se;
    ne->services.push_back(se);
    //onNodeNewService(...);
    rsp.ok = true;
    log(info, "Node '%s' announced service '%s' (%s)", ne->name, rq.service_name, rq.sock_addr);
}

void Resolver::handleResolveService(const b0::message::ResolveServiceRequest &rq, b0::message::ResolveServiceResponse &rsp)
{
    auto it = services_by_name_.find(rq.service_name);
    if(it == services_by_name_.end())
    {
        rsp.sock_addr = "";
        rsp.ok = false;
        log(error, "Failed to resolve service '%s'", rq.service_name);
        return;
    }
    resolver::ServiceEntry *se = it->second;
    log(trace, "Resolution: '%s' -> %s", rq.service_name, se->addr);
    rsp.ok = true;
    rsp.sock_addr = se->addr;
}

void Resolver::handleHeartbeat(const b0::message::HeartbeatRequest &rq, b0::message::HeartbeatResponse &rsp)
{
    if(rq.node_name == "resolver")
    {
        // a HeartbeatRequest from "resolver" means to actually perform
        // the detection and purging of dead nodes
        std::set<std::string> nodes_shutdown;
        for(auto i = nodes_by_name_.begin(); i != nodes_by_name_.end(); ++i)
        {
            resolver::NodeEntry *e = i->second;
            bool is_alive = (boost::posix_time::second_clock::local_time() - e->last_heartbeat) < boost::posix_time::seconds{5};
            if(!is_alive && e->name != this->getName())
                nodes_shutdown.insert(e->name);
        }
        for(auto node_name : nodes_shutdown)
        {
            log(info, "Node '%s' disconnected due to timeout.", node_name);
            resolver::NodeEntry *e = nodeByName(node_name);
            onNodeDisconnected(node_name);
            delete e;
        }
    }
    else
    {
        resolver::NodeEntry *ne = nodeByName(rq.node_name);
        if(!ne)
        {
            rsp.ok = false;
            log(error, "Received a heartbeat from an invalid node name: %s", rq.node_name);
            return;
        }
        heartbeat(ne);
    }
    rsp.ok = true;
    rsp.time_usec = hardwareTimeUSec();
}

void Resolver::handleNodeTopic(const b0::message::NodeTopicRequest &req, b0::message::NodeTopicResponse &resp)
{
    size_t old_sz1 = node_publishes_topic_.size(), old_sz2 = node_subscribes_topic_.size();
    if(req.reverse)
    {
        if(req.active)
            onNodeTopicSubscribeStart(req.node_name, req.topic_name);
        else
            onNodeTopicSubscribeStop(req.node_name, req.topic_name);
    }
    else
    {
        if(req.active)
            onNodeTopicPublishStart(req.node_name, req.topic_name);
        else
            onNodeTopicPublishStop(req.node_name, req.topic_name);
    }
    if(old_sz1 != node_publishes_topic_.size() || old_sz2 != node_subscribes_topic_.size())
        onGraphChanged();
}

void Resolver::handleNodeService(const b0::message::NodeServiceRequest &req, b0::message::NodeServiceResponse &resp)
{
    size_t old_sz1 = node_offers_service_.size(), old_sz2 = node_uses_service_.size();
    if(req.reverse)
    {
        if(req.active)
            onNodeServiceUseStart(req.node_name, req.service_name);
        else
            onNodeServiceUseStop(req.node_name, req.service_name);
    }
    else
    {
        if(req.active)
            onNodeServiceOfferStart(req.node_name, req.service_name);
        else
            onNodeServiceOfferStop(req.node_name, req.service_name);
    }
    if(old_sz1 != node_offers_service_.size() || old_sz2 != node_uses_service_.size())
        onGraphChanged();
}

void Resolver::handleGetGraph(const b0::message::GetGraphRequest &req, b0::message::GetGraphResponse &resp)
{
    getGraph(resp.graph);
}

void Resolver::getGraph(b0::message::Graph &graph)
{
    for(auto x : nodes_by_name_)
    {
        graph.nodes.push_back(x.first);
    }
    for(auto x : node_publishes_topic_)
    {
        b0::message::GraphLink l;
        l.node_name = x.first;
        l.other_name = x.second;
        l.reversed = false;
        graph.node_topic.push_back(l);
    }
    for(auto x : node_subscribes_topic_)
    {
        b0::message::GraphLink l;
        l.node_name = x.first;
        l.other_name = x.second;
        l.reversed = true;
        graph.node_topic.push_back(l);
    }
    for(auto x : node_offers_service_)
    {
        b0::message::GraphLink l;
        l.node_name = x.first;
        l.other_name = x.second;
        l.reversed = false;
        graph.node_service.push_back(l);
    }
    for(auto x : node_uses_service_)
    {
        b0::message::GraphLink l;
        l.node_name = x.first;
        l.other_name = x.second;
        l.reversed = true;
        graph.node_service.push_back(l);
    }
}

void Resolver::onGraphChanged()
{
    b0::message::Graph g;
    getGraph(g);
    graph_pub_.publish(g);
}

void Resolver::heartbeatSweeper()
{
    set_thread_name("HBsweep");

    resolver::Client resolv_cli(this);
    resolv_cli.setRemoteAddress("inproc://resolv");
    resolv_cli.init();

    while(!shutdownRequested())
    {
        // send a heartbeat to resolv itself trigger the sweeping:
        resolv_cli.sendHeartbeat(nullptr);
        boost::this_thread::sleep_for(boost::chrono::milliseconds{500});
    }

    resolv_cli.cleanup();
}

} // namespace resolver

} // namespace b0


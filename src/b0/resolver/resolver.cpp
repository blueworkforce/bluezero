#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <b0/utils/protobufhelpers.hpp>
#include <b0/resolver/resolver.h>
#include "resolver.pb.h"

namespace b0
{

namespace resolver
{

ResolverServiceServer::ResolverServiceServer(Resolver *resolver)
    : ServiceServer<b0::resolver_msgs::Request, b0::resolver_msgs::Response, false>(resolver, "resolver", &Resolver::handle),
      resolver_(resolver)
{
}

void ResolverServiceServer::announce()
{
    b0::resolver_msgs::AnnounceServiceRequest rq;
    node_.getNodeID(*rq.mutable_node_id());
    rq.set_service_name(service_name_);
    rq.set_sock_addr(remote_addr_);
    b0::resolver_msgs::AnnounceServiceResponse rsp;
    resolver_->handleAnnounceService(rq, rsp);
    resolver_->onNodeServiceOfferStart(resolver_->getName(), service_name_);
}

Resolver::Resolver()
    : Node("resolver"),
      resolv_server_(this),
      graph_pub_(this, "graph")
{
}

Resolver::~Resolver()
{
    pub_proxy_thread_.interrupt();
    //pub_proxy_thread_.join(); // FIXME: this makes the process hang on quit
}

void Resolver::init()
{
    Node::init();

    resolv_server_.bind((boost::format("tcp://*:%s") % resolverPort()).str());
    resolv_server_.bind(resolverAddress() /* "inproc://resolv" */); // for socket to self

    // setup XPUB-XSUB proxy addresses
    // those will be sent to nodes in response to announce
    int xsub_proxy_port_ = freeTCPPort();
    xsub_proxy_addr_ = address(hostAddress(), xsub_proxy_port_);
    log(TRACE, "XSUB address is %s", xsub_proxy_addr_);
    int xpub_proxy_port_ = freeTCPPort();
    xpub_proxy_addr_ = address(hostAddress(), xpub_proxy_port_);
    log(TRACE, "XPUB address is %s", xpub_proxy_addr_);
    // run XPUB-XSUB proxy:
    pub_proxy_thread_ = boost::thread(&Resolver::pubProxy, this, xsub_proxy_port_, xpub_proxy_port_);

    // run heartbeat sweeper (to detect when nodes go offline):
    heartbeat_sweeper_thread_ = boost::thread(&Resolver::heartBeatSweeper, this);

    // we have to manually call this because graph_pub_ doesn't send graph notify:
    // (has to be disabled because resolver is a special kind of node)
    onNodeTopicPublishStart(getName(), graph_pub_.getTopicName());

    log(INFO, "Ready.");
}

void Resolver::shutdown()
{
    Node::shutdown();

    pub_proxy_thread_.interrupt();
    pub_proxy_thread_.join();
}

std::string Resolver::resolverAddress() const
{
    return "inproc://resolv";
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
    b0::resolver_msgs::AnnounceNodeRequest rq;
    getNodeID(*rq.mutable_node_id());
    rq.set_node_name(getName());
    b0::resolver_msgs::AnnounceNodeResponse rsp;
    handleAnnounceNode(rq, rsp);

    logger_.connect("inproc://xsub_proxy");
}

void Resolver::notifyShutdown()
{
    // directly route this call to the handler, otherwise it will cause a deadlock
#if 0
    b0::resolver_msgs::ShutdownNodeRequest rq;
    getNodeID(*rq.mutable_node_id());
    b0::resolver_msgs::ShutdownNodeResponse rsp;
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
    nodes_by_key_.erase(nodeKey(e));
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
    log(INFO, "Graph: node '%s' publishes on topic '%s'", node_name, topic_name);
    node_publishes_topic_.insert(std::make_pair(node_name, topic_name));
}

void Resolver::onNodeTopicPublishStop(std::string node_name, std::string topic_name)
{
    log(INFO, "Graph: node '%s' stops publishing on topic '%s'", node_name, topic_name);
    node_publishes_topic_.erase(std::make_pair(node_name, topic_name));
}

void Resolver::onNodeTopicSubscribeStart(std::string node_name, std::string topic_name)
{
    log(INFO, "Graph: node '%s' subscribes to topic '%s'", node_name, topic_name);
    node_subscribes_topic_.insert(std::make_pair(node_name, topic_name));
}

void Resolver::onNodeTopicSubscribeStop(std::string node_name, std::string topic_name)
{
    log(INFO, "Graph: node '%s' stops subscribing to topic '%s'", node_name, topic_name);
    node_subscribes_topic_.erase(std::make_pair(node_name, topic_name));
}

void Resolver::onNodeServiceOfferStart(std::string node_name, std::string service_name)
{
    log(INFO, "Graph: node '%s' offers service '%s'", node_name, service_name);
    node_offers_service_.insert(std::make_pair(node_name, service_name));
}

void Resolver::onNodeServiceOfferStop(std::string node_name, std::string service_name)
{
    log(INFO, "Graph: node '%s' stops offering service '%s'", node_name, service_name);
    node_offers_service_.erase(std::make_pair(node_name, service_name));
}

void Resolver::onNodeServiceUseStart(std::string node_name, std::string service_name)
{
    log(INFO, "Graph: node '%s' connects to service '%s'", node_name, service_name);
    node_uses_service_.insert(std::make_pair(node_name, service_name));
}

void Resolver::onNodeServiceUseStop(std::string node_name, std::string service_name)
{
    log(INFO, "Graph: node '%s' disconnects from service '%s'", node_name, service_name);
    node_uses_service_.erase(std::make_pair(node_name, service_name));
}

void Resolver::pubProxy(int xsub_proxy_port, int xpub_proxy_port)
{
    zmq::socket_t proxy_in_sock_(context_, ZMQ_XSUB);
    std::string xsub_proxy_addr = address(xsub_proxy_port);
    log(DEBUG, "Binding XSUB socket to %s", xsub_proxy_addr);
    proxy_in_sock_.bind(xsub_proxy_addr);
    proxy_in_sock_.bind("inproc://xsub_proxy");

    zmq::socket_t proxy_out_sock_(context_, ZMQ_XPUB);
    std::string xpub_proxy_addr = address(xpub_proxy_port);
    log(DEBUG, "Binding XPUB socket to %s", xpub_proxy_addr);
    proxy_out_sock_.bind(xpub_proxy_addr);
    proxy_out_sock_.bind("inproc://xpub_proxy");

    try
    {
        log(TRACE, "Running XSUB/XPUB proxy...");
#ifdef __GNUC__
        zmq::proxy(static_cast<void*>(proxy_in_sock_), static_cast<void*>(proxy_out_sock_), nullptr);
#else
        zmq::proxy(proxy_in_sock_, proxy_out_sock_, nullptr);
#endif
        log(INFO, "XSUB/XPUB proxy has terminated");
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
    const char *host_addr = std::getenv("BWF_HOST_ID");
    if(host_addr) return host_addr;
    else return boost::asio::ip::host_name();
}

int Resolver::resolverPort() const
{
    const char *resolver_port = std::getenv("BWF_RESOLVER_PORT");
    if(!resolver_port) return 22000;
    return boost::lexical_cast<int>(resolver_port);
}

std::string Resolver::address(std::string host, int port)
{
    static boost::format f("tcp://%s:%d");
    return (f % host % port).str();
}

std::string Resolver::address(int port)
{
    return address("*", port);
}

std::string Resolver::nodeKey(std::string host_id, int process_id, std::string thread_id)
{
    return (boost::format("%s-%d-%s") % host_id % process_id % thread_id).str();
}

std::string Resolver::nodeKey(const b0::resolver_msgs::NodeID &node_id)
{
    return nodeKey(node_id.host_id(), node_id.process_id(), node_id.thread_id());
}

std::string Resolver::nodeKey(const resolver::NodeEntry *node_entry)
{
    return nodeKey(node_entry->host_id, node_entry->process_id, node_entry->thread_id);
}

resolver::NodeEntry * Resolver::nodeByID(const b0::resolver_msgs::NodeID &node_id)
{
    std::string k = nodeKey(node_id);
    auto it = nodes_by_key_.find(k);
    return it == nodes_by_key_.end() ? 0 : it->second;
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

void Resolver::heartBeat(resolver::NodeEntry *node_entry)
{
    node_entry->last_heartbeat = boost::posix_time::second_clock::local_time();
}

void Resolver::handle(const b0::resolver_msgs::Request &req, b0::resolver_msgs::Response &resp)
{
    if(req.has_announce_node())
        handleAnnounceNode(req.announce_node(), *resp.mutable_announce_node());
    else if(req.has_shutdown_node())
        handleShutdownNode(req.shutdown_node(), *resp.mutable_shutdown_node());
    else if(req.has_announce_service())
        handleAnnounceService(req.announce_service(), *resp.mutable_announce_service());
    else if(req.has_resolve())
        handleResolveService(req.resolve(), *resp.mutable_resolve());
    else if(req.has_heartbeat())
        handleHeartBeat(req.heartbeat(), *resp.mutable_heartbeat());
    else if(req.has_node_topic())
        handleNodeTopic(req.node_topic(), *resp.mutable_node_topic());
    else if(req.has_node_service())
        handleNodeService(req.node_service(), *resp.mutable_node_service());
    else if(req.has_get_graph())
        handleGetGraph(req.get_graph(), *resp.mutable_get_graph());
    else
        std::cerr << "resolver: received an unrecognized request" << std::endl;
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

void Resolver::handleAnnounceNode(const b0::resolver_msgs::AnnounceNodeRequest &rq, b0::resolver_msgs::AnnounceNodeResponse &rsp)
{
    log(TRACE, "Received a AnnounceNodeRequest");
    const b0::resolver_msgs::NodeID &node_id = rq.node_id();
    if(nodeByID(node_id))
    {
        rsp.set_ok(false);
        log(ERROR, "A node with the same id (%s) already exists", nodeKey(rq.node_id()));
        return;
    }
    std::string nodeName = makeUniqueNodeName(rq.node_name());
    resolver::NodeEntry *e = new resolver::NodeEntry;
    e->host_id = node_id.host_id();
    e->process_id = node_id.process_id();
    e->thread_id = node_id.thread_id();
    e->name = nodeName;
    heartBeat(e);
    nodes_by_name_[nodeName] = e;
    std::string key = nodeKey(e);
    nodes_by_key_[key] = e;
    onNodeConnected(nodeName);
    onGraphChanged();
    rsp.set_node_name(e->name);
    rsp.set_xsub_sock_addr(xsub_proxy_addr_);
    rsp.set_xpub_sock_addr(xpub_proxy_addr_);
    rsp.set_ok(true);
    log(INFO, "New node has joined: '%s' (key=%s)", e->name, key);
}

void Resolver::handleShutdownNode(const b0::resolver_msgs::ShutdownNodeRequest &rq, b0::resolver_msgs::ShutdownNodeResponse &rsp)
{
    resolver::NodeEntry *ne = nodeByID(rq.node_id());
    if(!ne)
    {
        rsp.set_ok(false);
        log(ERROR, "Invalid node id: %s", nodeKey(rq.node_id()));
        return;
    }
    std::string node_name = ne->name;
    onNodeDisconnected(node_name);
    delete ne;
    rsp.set_ok(true);
    log(INFO, "Node '%s' has left", node_name);
}

void Resolver::handleAnnounceService(const b0::resolver_msgs::AnnounceServiceRequest &rq, b0::resolver_msgs::AnnounceServiceResponse &rsp)
{
    resolver::NodeEntry *ne = nodeByID(rq.node_id());
    if(!ne)
    {
        rsp.set_ok(false);
        log(ERROR, "Invalid node id: %s", nodeKey(rq.node_id()));
        return;
    }
    if(serviceByName(rq.service_name()))
    {
        rsp.set_ok(false);
        log(ERROR, "A service with name '%s' already exists", rq.service_name());
        return;
    }
    resolver::ServiceEntry *se = new resolver::ServiceEntry;
    se->node = ne;
    se->name = rq.service_name();
    se->addr = rq.sock_addr();
    services_by_name_[se->name] = se;
    ne->services.push_back(se);
    //onNodeNewService(...);
    rsp.set_ok(true);
    log(INFO, "Node '%s' announced service '%s' (%s)", ne->name, rq.service_name(), rq.sock_addr());
}

void Resolver::handleResolveService(const b0::resolver_msgs::ResolveServiceRequest &rq, b0::resolver_msgs::ResolveServiceResponse &rsp)
{
    auto it = services_by_name_.find(rq.service_name());
    if(it == services_by_name_.end())
    {
        rsp.set_ok(false);
        log(ERROR, "Failed to resolve service '%s'", rq.service_name());
        return;
    }
    resolver::ServiceEntry *se = it->second;
    log(TRACE, "Resolution: '%s' -> %s", rq.service_name(), se->addr);
    rsp.set_ok(true);
    rsp.set_sock_addr(se->addr);
}

void Resolver::handleHeartBeat(const b0::resolver_msgs::HeartBeatRequest &rq, b0::resolver_msgs::HeartBeatResponse &rsp)
{
    if(rq.node_id().host_id() == "self" && rq.node_id().process_id() == 0 && rq.node_id().thread_id() == "self")
    {
        // a HeartBeatRequest from "self" pid=0 thread="self" means to actually perform
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
            log(INFO, "Node '%s' disconnected due to timeout.", node_name);
            resolver::NodeEntry *e = nodeByName(node_name);
            onNodeDisconnected(node_name);
            delete e;
        }
    }
    else
    {
        resolver::NodeEntry *ne = nodeByID(rq.node_id());
        if(!ne)
        {
            rsp.set_ok(false);
            log(ERROR, "Received a heartbeat from an invalid node id: %s", nodeKey(rq.node_id()));
            return;
        }
        heartBeat(ne);
    }
    rsp.set_ok(true);
    static boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
    rsp.set_time_usec((boost::posix_time::microsec_clock::local_time() - epoch).total_microseconds());
}

void Resolver::handleNodeTopic(const b0::resolver_msgs::NodeTopicRequest &req, b0::resolver_msgs::NodeTopicResponse &resp)
{
    size_t old_sz1 = node_publishes_topic_.size(), old_sz2 = node_subscribes_topic_.size();
    if(req.reverse())
    {
        if(req.active())
            onNodeTopicSubscribeStart(req.node_name(), req.topic_name());
        else
            onNodeTopicSubscribeStop(req.node_name(), req.topic_name());
    }
    else
    {
        if(req.active())
            onNodeTopicPublishStart(req.node_name(), req.topic_name());
        else
            onNodeTopicPublishStop(req.node_name(), req.topic_name());
    }
    if(old_sz1 != node_publishes_topic_.size() || old_sz2 != node_subscribes_topic_.size())
        onGraphChanged();
}

void Resolver::handleNodeService(const b0::resolver_msgs::NodeServiceRequest &req, b0::resolver_msgs::NodeServiceResponse &resp)
{
    size_t old_sz1 = node_offers_service_.size(), old_sz2 = node_uses_service_.size();
    if(req.reverse())
    {
        if(req.active())
            onNodeServiceUseStart(req.node_name(), req.service_name());
        else
            onNodeServiceUseStop(req.node_name(), req.service_name());
    }
    else
    {
        if(req.active())
            onNodeServiceOfferStart(req.node_name(), req.service_name());
        else
            onNodeServiceOfferStop(req.node_name(), req.service_name());
    }
    if(old_sz1 != node_offers_service_.size() || old_sz2 != node_uses_service_.size())
        onGraphChanged();
}

void Resolver::handleGetGraph(const b0::resolver_msgs::GetGraphRequest &req, b0::resolver_msgs::GetGraphResponse &resp)
{
    getGraph(*resp.mutable_graph());
}

void Resolver::getGraph(b0::resolver_msgs::Graph &graph)
{
    for(auto x : nodes_by_name_)
    {
        graph.add_nodes(x.first);
    }
    for(auto x : node_publishes_topic_)
    {
        b0::resolver_msgs::GraphLink *l = graph.add_node_topic();
        l->set_a(x.first);
        l->set_b(x.second);
        l->set_reversed(false);
    }
    for(auto x : node_subscribes_topic_)
    {
        b0::resolver_msgs::GraphLink *l = graph.add_node_topic();
        l->set_a(x.first);
        l->set_b(x.second);
        l->set_reversed(true);
    }
    for(auto x : node_offers_service_)
    {
        b0::resolver_msgs::GraphLink *l = graph.add_node_service();
        l->set_a(x.first);
        l->set_b(x.second);
        l->set_reversed(false);
    }
    for(auto x : node_uses_service_)
    {
        b0::resolver_msgs::GraphLink *l = graph.add_node_service();
        l->set_a(x.first);
        l->set_b(x.second);
        l->set_reversed(true);
    }
}

void Resolver::onGraphChanged()
{
    b0::resolver_msgs::Graph g;
    getGraph(g);
    graph_pub_.publish(g);
}

void Resolver::heartBeatSweeper()
{
    zmq::socket_t socket(context_, ZMQ_REQ);
    socket.connect("inproc://resolv");

    while(!shutdownRequested())
    {
        // send a special heartbeat to resolv itself trigger the sweeping:

        b0::resolver_msgs::Request rq0;
        b0::resolver_msgs::HeartBeatRequest &rq = *rq0.mutable_heartbeat();
        rq.mutable_node_id()->set_host_id("self");
        rq.mutable_node_id()->set_process_id(0);
        rq.mutable_node_id()->set_thread_id("self");
        s_send(socket, rq0);

        b0::resolver_msgs::Response rsp0;
        s_recv(socket, rsp0);
        const b0::resolver_msgs::HeartBeatResponse &rsp = rsp0.heartbeat();
        if(!rsp.ok()) break;

        boost::this_thread::sleep_for(boost::chrono::milliseconds{500});
    }

    // don't call log() from another thread! leave the following commented out:
    //log(INFO, "Heartbeat sweeper thread terminates.");
}

} // namespace resolver

} // namespace b0


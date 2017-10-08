#ifndef RESOLVER_H_INCLUDED
#define RESOLVER_H_INCLUDED

#include <b0/node.h>
#include <b0/service_server.h>
#include <b0/publisher.h>

#include <string>
#include <vector>
#include <boost/thread.hpp>
#include <boost/format.hpp>

namespace b0
{

//! \cond HIDDEN_SYMBOLS

namespace resolver
{

struct ServiceEntry;

struct NodeEntry
{
    std::string host_id;
    int process_id;
    std::string thread_id;
    std::string name;
    boost::posix_time::ptime last_heartbeat;
    std::vector<ServiceEntry*> services;
};

struct ServiceEntry
{
    NodeEntry *node;
    std::string name;
    std::string addr;
};

} // namespace resolver

class Resolver;

class ResolverServiceServer : public ServiceServer<b0::resolver_msgs::Request, b0::resolver_msgs::Response, false>
{
public:
    ResolverServiceServer(Resolver *resolver);

protected:
    /*!
     * \brief Hijack the announce step
     */
    virtual void announce() override;

    //! \brief Pointer to resolver node
    Resolver *resolver_;
};

//! \endcond

/*!
 * \brief The resolver node
 */
class Resolver : public Node
{
public:
    /*!
     * \brief Construct a resolver node
     */
    Resolver();

    /*!
     * \brief Resolver node destructor
     */
    virtual ~Resolver();

    /*!
     * \brief Perform node initialization
     */
    void init() override;

    /*!
     * \brief Provide the (inproc) resolver address
     */
    std::string resolverAddress() const override;

    /*!
     * \brief Retrieve address of the proxy's XPUB socket
     */
    virtual std::string getXPUBSocketAddress() const override;

    /*!
     * \brief Retrieve address of the proxy's XSUB socket
     */
    virtual std::string getXSUBSocketAddress() const override;

    /*!
     * \brief Hijack announceNode step
     */
    virtual void announceNode() override;

    /*!
     * Called when a new node has connected
     */
    void onNodeConnected(std::string name);

    /*!
     * Called when a node disconnects (detected with heartbeat timeout)
     */
    void onNodeDisconnected(std::string name);

    /*!
     * \brief The XSUB/XPUB proxy (will be started in a separate thread)
     */
    void pubProxy(int xsub_proxy_port, int xpub_proxy_port);

    /*!
     * \brief Checks wether a node with this name exists in the connected nodes list
     */
    bool nodeNameExists(std::string name);

    /*!
     * \brief Return this host name or IP address
     */
    virtual std::string hostAddress();

    /*!
     * \brief Return the port number of the resolver socket
     */
    virtual int resolverPort() const;

    /*!
     * \brief Format a tcp:// address
     */
    virtual std::string address(std::string host, int port);

    /*!
     * \brief Format a tcp:// bind address
     */
    virtual std::string address(int port);

    /*!
     * \brief Create a string with the node key
     */
    std::string nodeKey(std::string host_id, int process_id, std::string thread_id);

    /*!
     * \brief Create a string with the node key
     */
    std::string nodeKey(const b0::resolver_msgs::NodeID &node_id);

    /*!
     * \brief Create a string with the node key
     */
    std::string nodeKey(const resolver::NodeEntry *node_entry);

    /*!
     * \brief Get the NodeEntry given the NodeID
     */
    virtual resolver::NodeEntry * nodeByID(const b0::resolver_msgs::NodeID &node_id);

    /*!
     * \brief Get the NodeEntry given the node name
     */
    virtual resolver::NodeEntry * nodeByName(std::string node_name);

    /*!
     * \brief Get the ServiceEntry given the service name
     */
    virtual resolver::ServiceEntry * serviceByName(std::string service_name);

    /*!
     * \brief Update the NodeEntry timestamp
     */
    virtual void heartBeat(resolver::NodeEntry *node_entry);

    /*!
     * \brief Handle a service on the resolv service
     */
    virtual void handle(const b0::resolver_msgs::Request &req, b0::resolver_msgs::Response &resp);

    /*!
     * \brief Adjust nodeName such that it is unique in the network (amongst the list of connected nodes)
     */
    std::string makeUniqueNodeName(std::string nodeName);

    /*!
     * \brief Handle the AnnounceNode request
     */
    virtual void handleAnnounceNode(const b0::resolver_msgs::AnnounceNodeRequest &rq, b0::resolver_msgs::AnnounceNodeResponse &rsp);

    /*!
     * \brief Handle the AnnounceService request
     */
    virtual void handleAnnounceService(const b0::resolver_msgs::AnnounceServiceRequest &rq, b0::resolver_msgs::AnnounceServiceResponse &rsp);

    /*!
     * \brief Handle the ResolveService request
     */
    virtual void handleResolveService(const b0::resolver_msgs::ResolveServiceRequest &rq, b0::resolver_msgs::ResolveServiceResponse &rsp);

    /*!
     * \brief Handle the HeartBeat request
     */
    virtual void handleHeartBeat(const b0::resolver_msgs::HeartBeatRequest &rq, b0::resolver_msgs::HeartBeatResponse &rsp);

    /*!
     * \brief Handle the NodeTopic request
     */
    void handleNodeTopic(const b0::resolver_msgs::NodeTopicRequest &req, b0::resolver_msgs::NodeTopicResponse &resp);

    /*!
     * \brief Handle the NodeService request
     */
    void handleNodeService(const b0::resolver_msgs::NodeServiceRequest &req, b0::resolver_msgs::NodeServiceResponse &resp);

    /*!
     * \brief Handle the GetGraph request
     */
    void handleGetGraph(const b0::resolver_msgs::GetGraphRequest &req, b0::resolver_msgs::GetGraphResponse &resp);

    /*!
     * Retrieve the current Graph
     */
    void getGraph(b0::resolver_msgs::Graph &graph);

    /*!
     * \brief Called when the global graph changes
     *
     * Due to a node publishing or subscribing a topic, or offering or using a service.
     *
     */
    void onGraphChanged();

    /*!
     * \brief Code to run in the heartbeat sweeper thread
     */
    void heartBeatSweeper();

protected:
    //! The ServiceServer serving the requests for the resolv protocol
    ResolverServiceServer resolv_server_;

    //! Public address of the XSUB socket of the ZeroMQ proxy
    std::string xsub_proxy_addr_;

    //! Public address of the XPUB socket of the ZeroMQ proxy
    std::string xpub_proxy_addr_;

    //! The thread running the ZeroMQ XSUB/XPUB proxy
    boost::thread pub_proxy_thread_;

    //! The heartbeat sweeper thread
    boost::thread heartbeat_sweeper_thread_;

    //! Map of nodes by name
    std::map<std::string, resolver::NodeEntry*> nodes_by_name_;

    //! Map of nodes by key
    std::map<std::string, resolver::NodeEntry*> nodes_by_key_;

    //! Map of services by name
    std::map<std::string, resolver::ServiceEntry*> services_by_name_;

    //! Graph edges node <--> topic
    std::map<std::pair<std::string, std::string>, bool> node_topic_;
    //
    //! Graph edges node <--> service
    std::map<std::pair<std::string, std::string>, bool> node_service_;

    //! Publisher of the Graph message
    b0::Publisher<b0::resolver_msgs::Graph, false> graph_pub_;
};

} // namespace b0

#endif // RESOLVER_H_INCLUDED

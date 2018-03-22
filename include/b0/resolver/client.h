#ifndef B0__RESOLVER__CLIENT_H__INCLUDED
#define B0__RESOLVER__CLIENT_H__INCLUDED

#include <b0/protobuf/service_client.h>

#include <cstdint>
#include <string>

namespace b0
{

class Node;

namespace resolver_msgs
{

class Request;
class Response;
class Graph;

} // namespace resolver_msgs

namespace resolver
{

/*!
 * \brief The resolver client
 *
 * Performs service name resolution.
 */
class Client : public protobuf::ServiceClient<b0::resolver_msgs::Request, b0::resolver_msgs::Response>
{
public:
    /*!
     * \brief Resolver client constructr
     */
    Client(b0::Node *node);

    /*!
     * \brief Resolver client destructor
     */
    virtual ~Client();

    /*!
     * \brief Set a timeout for the read in the announce phase. Use -1 for no timeout.
     * A timeout will cause the announce phase to abort if a response from the resolver
     * node is not received within the specified amount of milliseconds.
     */
    void setAnnounceTimeout(int timeout = -1);

    /*!
     * \brief Announce this node to resolver
     */
    virtual void announceNode(std::string &node_name, std::string &xpub_sock_addr, std::string &xsub_sock_addr);

    /*!
     * \brief Notify resolver of this node shutdown
     */
    virtual void notifyShutdown();
    
    /*!
     * \brief Send a heartbeat to resolver
     */
    virtual void sendHeartbeat(int64_t *time_usec = nullptr);

    /*!
     * \brief Notify topic publishing/subscription start or end
     */
    virtual void notifyTopic(std::string topic_name, bool reverse, bool active);

    /*!
     * \brief Notify service advertising/use start or end
     */
    virtual void notifyService(std::string service_name, bool reverse, bool active);

    /*!
     * \brief Announce a service name and address
     */
    virtual void announceService(std::string name, std::string addr);

    /*!
     * \brief Resolve a service name
     */
    virtual void resolveService(std::string name, std::string &addr);

    /*!
     * \brief Request the node sockets graph
     */
    virtual void getGraph(b0::resolver_msgs::Graph &graph);

private:
    int announce_timeout_;
};

} // namespace resolver

} // namespace b0

#endif // B0__RESOLVER__CLIENT_H__INCLUDED

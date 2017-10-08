#ifndef SERVICE_CLIENT_H_INCLUDED
#define SERVICE_CLIENT_H_INCLUDED

#include <b0/node.h>
#include <b0/graph.h>

namespace b0
{

//! \cond HIDDEN_SYMBOLS

class AbstractServiceClient
{
public:
    AbstractServiceClient(Node *node, std::string service_name)
        : node_(*node),
          service_name_(service_name),
          req_socket_(node_.getZMQContext(), ZMQ_REQ)
    {
        node_.addServiceClient(this);
    }

    virtual ~AbstractServiceClient()
    {
        node_.removeServiceClient(this);
    }

    virtual void init()
    {
        resolve();
        connect();
    }

protected:
    /*!
     * \brief Resolve the name of a service to a ZeroMQ address
     */
    void resolve()
    {
        zmq::socket_t &resolv_socket = node_.resolverSocket();

        b0::resolver_msgs::Request rq0;
        b0::resolver_msgs::ResolveServiceRequest &rq = *rq0.mutable_resolve();
        rq.set_service_name(service_name_);
        s_send(resolv_socket, rq0);

        b0::resolver_msgs::Response rsp0;
        s_recv(resolv_socket, rsp0);
        const b0::resolver_msgs::ResolveServiceResponse &rsp = rsp0.resolve();
        remote_addr_ = rsp.sock_addr();
        node_.log(node_.TRACE, "Resolve %s -> %s", service_name_, remote_addr_);
    }

    /*!
     * \brief Connect to the specified service
     */
    void connect()
    {
        node_.log(node_.TRACE, "Connecting to service '%s' (%s)...", service_name_, remote_addr_);
        req_socket_.connect(remote_addr_);
    }

    //! The Node owning this ServiceClient
    Node &node_;

    //! The name of the service
    std::string service_name_;

    //! The ZeroMQ REQ socket
    zmq::socket_t req_socket_;

    //! The address of the ZeroMQ socket in the server
    std::string remote_addr_;
};

//! \endcond

/*!
 * \brief Service client
 *
 * This class wraps a ZeroMQ REQ socket. It will use Node::connectSocket() to
 * automatically resolve the node of the target socket (identified by node_name and service_name
 * passed to the constructor ServiceClient::ServiceClient()).
 *
 * The remote service is invoked with ServiceClient::call() and the call is blocking.
 * It will unblock as soon as the server sends out a reply.
 *
 * You can make it work asynchronously by directly using ServiceClient::write(), and polling
 * for the reply with ServiceClient::poll(), followed by ServiceClient::read().
 *
 * \sa ServiceServer
 */
template<typename TReq, typename TRep, bool notifyGraph = true>
class ServiceClient : public AbstractServiceClient
{
public:
    /*!
     * \brief Construct a ServiceClient child of a specific Node, which will connect to the specified socket in the specified node
     */
    ServiceClient(Node *node, std::string service_name)
        : AbstractServiceClient(node, service_name)
    {
    }

    /*!
     * \brief Write a request and read a reply from the underlying ZeroMQ REQ socket
     * \sa read(const TReq&), write(TRep&)
     */
    virtual void call(const TReq &req, TRep &rep)
    {
        write(req);
        read(rep);
    }

    /*!
     * \brief Write a request message to the underlying ZeroMQ REQ socket
     */
    virtual bool write(const TReq &req)
    {
        std::string payload;
        bool ret = req.SerializeToString(&payload);
        ::s_send(req_socket_, payload);
        return ret;
    }

    /*!
     * \brief Poll the underlying ZeroMQ REQ socket
     */
    virtual bool poll(long timeout = 0)
    {
#ifdef __GNUC__
        zmq::pollitem_t items[] = {{static_cast<void*>(req_socket_), 0, ZMQ_POLLIN, 0}};
#else
        zmq::pollitem_t items[] = {{req_socket_, 0, ZMQ_POLLIN, 0}};
#endif
        zmq::poll(&items[0], sizeof(items) / sizeof(items[0]), timeout);
        return items[0].revents & ZMQ_POLLIN;
    }

    /*!
     * \brief Read a reply message from the underlying ZeroMQ REQ socket
     */
    virtual bool read(TRep &rep)
    {
        std::string payload = ::s_recv(req_socket_);
        return rep.ParseFromString(payload);
    }

    void init() override
    {
        AbstractServiceClient::init();

        if(notifyGraph)
            b0::graph::notifyService(node_, service_name_, true, true);
    }
};

} // namespace b0

#endif // SERVICE_CLIENT_H_INCLUDED

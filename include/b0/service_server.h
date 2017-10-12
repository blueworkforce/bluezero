#ifndef SERVICE_SERVER_H_INCLUDED
#define SERVICE_SERVER_H_INCLUDED

#include <b0/node.h>
#include <b0/graph.h>

namespace b0
{

//! \cond HIDDEN_SYMBOLS

class AbstractServiceServer
{
public:
    AbstractServiceServer(Node *node, std::string service_name)
        : node_(*node),
          service_name_(service_name),
          rep_socket_(node_.getZMQContext(), ZMQ_REP),
          bind_addr_(""),
          remote_addr_("")
    {
        node_.addServiceServer(this);
    }

    virtual ~AbstractServiceServer()
    {
        node_.removeServiceServer(this);
    }

    virtual void init()
    {
        bind();
        announce();
    }

    virtual void cleanup()
    {
        unbind();
    }

    /*!
     * \brief Poll and read incoming messages, and dispatch them
     */
    virtual void spinOnce() = 0;

protected:
    /*!
     * \brief Bind this service socket
     */
    virtual void bind()
    {
        static boost::format fmt("tcp://%s:%d");
        std::string host = node_.hostname();
        int port = node_.freeTCPPort();
        bind_addr_ = (fmt % "*" % port).str();
        remote_addr_ = (fmt % host % port).str();
        rep_socket_.bind(bind_addr_);
        node_.log(node_.DEBUG, "Bound service '%s' to %s", service_name_, bind_addr_);
    }

    virtual void unbind()
    {
        //rep_socket_.unbind(bind_addr_); // FIXME: causes a zmq error on node shutdown
    }

    /*!
     * \brief Announce this service name to resolver
     */
    virtual void announce()
    {
        zmq::socket_t &resolv_socket = node_.resolverSocket();

        node_.log(node_.TRACE, "Announcing service '%s' (%s) to resolver...", service_name_, remote_addr_);
        b0::resolver_msgs::Request rq0;
        b0::resolver_msgs::AnnounceServiceRequest &rq = *rq0.mutable_announce_service();
        node_.getNodeID(*rq.mutable_node_id());
        rq.set_service_name(service_name_);
        rq.set_sock_addr(remote_addr_);
        s_send(resolv_socket, rq0);

        b0::resolver_msgs::Response rsp0;
        s_recv(resolv_socket, rsp0);
        const b0::resolver_msgs::AnnounceServiceResponse &rsp = rsp0.announce_service();
    }

    //! The Node owning this ServiceServer
    Node &node_;

    //! The name of the service exposed by this ServiceServer
    std::string service_name_;

    //! The ZeroMQ REP socket
    zmq::socket_t rep_socket_;

    //! The ZeroMQ address to bind the service socket on
    std::string bind_addr_;

    //! The remote ZeroMQ address to connect to this service socket
    std::string remote_addr_;
};

//! \endcond

/*!
 * \brief Service server
 *
 * This class wraps a ZeroMQ REP socket. It will use Node::bindSocket() to automatically
 * announce the socket name to resolver.
 *
 * If using a callback, when a request is received, it will be handed to the callback, as long as
 * a spin method is called (e.g. Node::spin(), Node::spinOnce() or ServiceServer::spinOnce()).
 *
 * You can directly read requests and write replies to the underlying socket, by using
 * ServiceServer::poll(), ServiceServer::read() and ServiceServer::write().
 *
 * \sa ServiceClient
 */
template<typename TReq, typename TRep, bool notifyGraph = true>
class ServiceServer : public AbstractServiceServer
{
public:
    /*!
     * \brief Construct a ServiceServer child of a specific Node, using a boost::function as callback
     */
    ServiceServer(Node *node, std::string service_name, boost::function<void(const TReq&, TRep&)> callback = 0)
        : AbstractServiceServer(node, service_name),
          callback_(callback)
    {
    }

    /*!
     * \brief Construct a ServiceServer child of a specific Node, using a method (of the Node subclass) as callback
     */
    template<class TNode>
    ServiceServer(TNode *node, std::string service_name, void (TNode::*callbackMethod)(const TReq&, TRep&))
        : ServiceServer(node, service_name, boost::bind(callbackMethod, node, _1, _2))
    {
        // delegate constructor. leave empty
    }

    /*!
     * \brief Construct a ServiceServer child of a specific Node, using a method as callback
     */
    template<class T>
    ServiceServer(Node *node, std::string service_name, void (T::*callbackMethod)(const TReq&, TRep&), T *callbackObject)
        : ServiceServer(node, service_name, boost::bind(callbackMethod, callbackObject, _1, _2))
    {
        // delegate constructor. leave empty
    }

    /*!
     * \brief Poll and read incoming messages, and dispatch them (called by b0::Node::spinOnce())
     */
    virtual void spinOnce() override
    {
        if(callback_.empty()) return;

        while(poll())
        {
            TReq req;
            TRep rep;
            if(read(req))
            {
                callback_(req, rep);
            }
            write(rep);
        }
    }

    /*!
     * \brief Bind the underlying ZeroMQ REP socket to an additional address
     *
     * Useful for adding an inproc:// address.
     */
    virtual void bind(std::string address)
    {
        rep_socket_.bind(address);
    }

    /*!
     * \brief Poll the underlying ZeroMQ REP socket
     */
    virtual bool poll(long timeout = 0)
    {
#ifdef __GNUC__
        zmq::pollitem_t items[] = {{static_cast<void*>(rep_socket_), 0, ZMQ_POLLIN, 0}};
#else
        zmq::pollitem_t items[] = {{rep_socket_, 0, ZMQ_POLLIN, 0}};
#endif
        zmq::poll(&items[0], sizeof(items) / sizeof(items[0]), timeout);
        return items[0].revents & ZMQ_POLLIN;
    }

    /*!
     * \brief Read a request message from the underlying ZeroMQ REP socket
     */
    virtual bool read(TReq &req)
    {
        std::string payload = ::s_recv(rep_socket_);
        return req.ParseFromString(payload);
    }

    /*!
     * \brief Write a reply message to the underlying ZeroMQ REP socket
     */
    virtual bool write(const TRep &rep)
    {
        std::string payload;
        bool ret = rep.SerializeToString(&payload);
        ::s_send(rep_socket_, payload);
        return ret;
    }

    /*!
     * \brief Perform initialization and optionally send graph notify
     */
    void init() override
    {
        AbstractServiceServer::init();

        if(notifyGraph)
            b0::graph::notifyService(node_, service_name_, false, true);
    }

    /*!
     * \brief Perform cleanup and optionally send graph notify
     */
    void cleanup() override
    {
        AbstractServiceServer::cleanup();

        if(notifyGraph)
            b0::graph::notifyService(node_, service_name_, false, false);
    }

protected:
    /*!
     * \brief Callback which will be called when a new message is read from the socket
     */
    boost::function<void(const TReq&, TRep&)> callback_;
};

} // namespace b0

#endif // SERVICE_SERVER_H_INCLUDED

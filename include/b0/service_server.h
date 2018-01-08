#ifndef B0__SERVICE_SERVER_H__INCLUDED
#define B0__SERVICE_SERVER_H__INCLUDED

#include <string>

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <b0/socket/socket.h>

namespace b0
{

class Node;

/*!
 * \brief The (abstract) service server class
 *
 * This class wraps a ZeroMQ REP socket. It will automatically
 * announce the socket name to resolver.
 *
 * \sa b0::ServiceClient, b0::ServiceServer, b0::AbstractServiceClient, b0::AbstractServiceServer
 */
class AbstractServiceServer : public socket::Socket
{
public:
    using logger::LogInterface::log;

    /*!
     * \brief Construct an AbstractServiceServer child of the specified Node
     */
    AbstractServiceServer(Node *node, std::string service_name, bool managed = true, bool notify_graph = true);

    /*!
     * \brief AbstractServiceServer destructor
     */
    virtual ~AbstractServiceServer();

    /*!
     * \brief Log a message using node's logger, prepending this service server informations
     */
    void log(LogLevel level, std::string message) const override;

    /*!
     * \brief Perform initialization and optionally send graph notify
     */
    virtual void init() override;

    /*!
     * \brief Perform cleanup and optionally send graph notify
     */
    virtual void cleanup() override;

    /*!
     * \brief Return the name of this server's service
     */
    std::string getServiceName();

    /*!
     * \brief Bind to an additional address
     */
    virtual void bind(std::string address);

protected:
    /*!
     * \brief Bind socket to the address
     */
    virtual void bind();

    /*!
     * \brief Unbind socket from the address
     */
    virtual void unbind();

    /*!
     * \brief Announce service to resolver
     */
    virtual void announce();

    //! The ZeroMQ address to bind the service socket on
    std::string bind_addr_;

    //! If false this socket will not send announcement to resolv (i.e. it will be "invisible")
    const bool notify_graph_;
};

/*!
 * \brief The service server template class
 *
 * This template class specializes b0::AbstractServiceClient to a specific request/response type.
 *
 * If using a callback, when a request is received, it will be handed to the callback, as long as
 * a spin method is called (e.g. Node::spin(), Node::spinOnce() or ServiceServer::spinOnce()).
 *
 * You can directly read requests and write replies from the underlying socket, by using
 * ServiceServer::poll(), ServiceServer::read() and ServiceServer::write().
 *
 * \sa b0::ServiceClient, b0::ServiceServer, b0::AbstractServiceClient, b0::AbstractServiceServer
 */
template<typename TReq, typename TRep>
class ServiceServer : public AbstractServiceServer
{
public:
    /*!
     * \brief Construct a ServiceServer child of a specific Node, using a boost::function as callback
     */
    ServiceServer(Node *node, std::string service_name, boost::function<void(const TReq&, TRep&)> callback = 0, bool managed = true, bool notify_graph = true)
        : AbstractServiceServer(node, service_name, managed, notify_graph),
          callback_(callback)
    {
    }

    /*!
     * \brief Construct a ServiceServer child of a specific Node, using a method (of the Node subclass) as callback
     */
    template<class TNode>
    ServiceServer(TNode *node, std::string service_name, void (TNode::*callbackMethod)(const TReq&, TRep&), bool managed = true, bool notify_graph = true)
        : ServiceServer(node, service_name, boost::bind(callbackMethod, node, _1, _2), managed, notify_graph)
    {
        // delegate constructor. leave empty
    }

    /*!
     * \brief Construct a ServiceServer child of a specific Node, using a method as callback
     */
    template<class T>
    ServiceServer(Node *node, std::string service_name, void (T::*callbackMethod)(const TReq&, TRep&), T *callbackObject, bool managed = true, bool notify_graph = true)
        : ServiceServer(node, service_name, boost::bind(callbackMethod, callbackObject, _1, _2), managed, notify_graph)
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
            read(req);
            callback_(req, rep);
            write(rep);
        }
    }

protected:
    /*!
     * \brief Callback which will be called when a new message is read from the socket
     */
    boost::function<void(const TReq&, TRep&)> callback_;
};

/*!
 * \brief Raw version of ServiceServer::spinOnce()
 */
template<>
inline void ServiceServer<std::string, std::string>::spinOnce()
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

} // namespace b0

#endif // B0__SERVICE_SERVER_H__INCLUDED

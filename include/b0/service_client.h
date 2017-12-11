#ifndef B0__SERVICE_CLIENT_H__INCLUDED
#define B0__SERVICE_CLIENT_H__INCLUDED

#include <string>

#include <b0/socket/socket.h>

namespace b0
{

class Node;

/*!
 * \brief The (abstract) service client class
 *
 * This class wraps a ZeroMQ REQ socket. It will automatically resolve the address
 * of service name.
 *
 * \sa b0::ServiceClient, b0::ServiceServer, b0::AbstractServiceClient, b0::AbstractServiceServer
 */
class AbstractServiceClient : public socket::Socket
{
public:
    using logger::LogInterface::log;

    /*!
     * \brief Construct an AbstractServiceClient child of the specified Node
     */
    AbstractServiceClient(Node *node, std::string service_name, bool managed, bool notify_graph);

    /*!
     * \brief AbstractServiceClient destructor
     */
    virtual ~AbstractServiceClient();

    /*!
     * \brief Log a message using node's logger, prepending this service client informations
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
     * \brief Return the name of this client's service
     */
    std::string getServiceName();

protected:
    /*!
     * \brief Perform service address resolution
     */
    virtual void resolve();

    /*!
     * \brief Connect to service server endpoint
     */
    virtual void connect();

    /*!
     * \brief Disconnect from service server endpoint
     */
    virtual void disconnect();

    //! If false this socket will not send announcement to resolv (i.e. it will be "invisible")
    const bool notify_graph_;
};

/*!
 * \brief The service client template class
 *
 * This template class specializes b0::AbstractServiceClient to a specific request/response type.
 * It implements the call() method as well.
 *
 * The remote service is invoked with ServiceClient::call() and the call is blocking.
 * It will unblock as soon as the server sends out a reply.
 *
 * You can make it work asynchronously by directly using ServiceClient::write(), and polling
 * for the reply with ServiceClient::poll(), followed by ServiceClient::read().
 *
 * \sa b0::ServiceClient, b0::ServiceServer, b0::AbstractServiceClient, b0::AbstractServiceServer
 */
template<typename TReq, typename TRep>
class ServiceClient : public AbstractServiceClient
{
public:
    /*!
     * \brief Construct a ServiceClient child of a specific Node, which will connect to the specified socket in the specified node
     */
    ServiceClient(Node *node, std::string service_name, bool managed = true, bool notify_graph = true)
        : AbstractServiceClient(node, service_name, managed, notify_graph)
    {
    }

    /*!
     * \brief Write a request and read a reply from the underlying ZeroMQ REQ socket
     * \sa ServiceServer::read(), ServiceServer::write()
     */
    virtual void call(const TReq &req, TRep &rep)
    {
        write(req);
        read(rep);
    }
};

} // namespace b0

#endif // B0__SERVICE_CLIENT_H__INCLUDED

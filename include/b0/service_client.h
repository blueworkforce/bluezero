#ifndef B0__SERVICE_CLIENT_H__INCLUDED
#define B0__SERVICE_CLIENT_H__INCLUDED

#include <string>

#include <b0/socket/socket.h>

namespace b0
{

class Node;

//! \cond HIDDEN_SYMBOLS

class AbstractServiceClient : public socket::Socket
{
public:
    using logger::LogInterface::log;

    AbstractServiceClient(Node *node, std::string service_name, bool managed, bool notify_graph);

    virtual ~AbstractServiceClient();

    void log(LogLevel level, std::string message) const override;

    /*!
     * \brief Perform initialization and optionally send graph notify
     */
    virtual void init() override;

    /*!
     * \brief Perform cleanup and optionally send graph notify
     */
    virtual void cleanup() override;

    virtual void spinOnce() override {}

    std::string getServiceName();

protected:
    virtual void resolve();
    virtual void connect();
    virtual void disconnect();

    const bool notify_graph_;
};

//! \endcond

/*!
 * \brief Service client
 *
 * This class wraps a ZeroMQ REQ socket. It will automatically resolve the address
 * of the target socket (identified by the service_name argument passed to the
 * constructor ServiceClient::ServiceClient()).
 *
 * The remote service is invoked with ServiceClient::call() and the call is blocking.
 * It will unblock as soon as the server sends out a reply.
 *
 * You can make it work asynchronously by directly using ServiceClient::write(), and polling
 * for the reply with ServiceClient::poll(), followed by ServiceClient::read().
 *
 * \sa ServiceServer
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

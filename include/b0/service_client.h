#ifndef B0__SERVICE_CLIENT_H__INCLUDED
#define B0__SERVICE_CLIENT_H__INCLUDED

#include <string>

#include <b0/socket/socket.h>
#include <b0/graph/graph.h>

namespace b0
{

class Node;

//! \cond HIDDEN_SYMBOLS

class AbstractServiceClient : public socket::Socket
{
public:
    using logger::LogInterface::log;

    AbstractServiceClient(Node *node, std::string service_name, bool managed = true);
    virtual ~AbstractServiceClient();
    void log(LogLevel level, std::string message) const override;
    virtual void init() override;
    virtual void cleanup() override;
    virtual void spinOnce() override {}
    std::string getServiceName();

protected:
    virtual void resolve();
    virtual void connect();
    virtual void disconnect();
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
template<typename TReq, typename TRep, bool notifyGraph = true>
class ServiceClient : public AbstractServiceClient
{
public:
    /*!
     * \brief Construct a ServiceClient child of a specific Node, which will connect to the specified socket in the specified node
     */
    ServiceClient(Node *node, std::string service_name, bool managed = true)
        : AbstractServiceClient(node, service_name, managed)
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

    /*!
     * \brief Perform initialization and optionally send graph notify
     */
    void init() override
    {
        AbstractServiceClient::init();

        if(notifyGraph)
            b0::graph::notifyService(node_, name_, true, true);
    }

    /*!
     * \brief Perform cleanup and optionally send graph notify
     */
    void cleanup() override
    {
        AbstractServiceClient::cleanup();

        if(notifyGraph)
            b0::graph::notifyService(node_, name_, true, false);
    }
};

} // namespace b0

#endif // B0__SERVICE_CLIENT_H__INCLUDED

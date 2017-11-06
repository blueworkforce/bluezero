#ifndef B0__SERVICE_CLIENT_H__INCLUDED
#define B0__SERVICE_CLIENT_H__INCLUDED

#include <string>

#include <b0/utils/protobufhelpers.h>
#include <b0/graph/graph.h>

namespace b0
{

class Node;

//! \cond HIDDEN_SYMBOLS

class AbstractServiceClient
{
public:
    AbstractServiceClient(Node *node, std::string service_name, bool managed = true);
    virtual ~AbstractServiceClient();
    void setCompression(std::string algorithm, int level = -1);
    void setRemoteAddress(std::string addr);
    virtual void init();
    virtual void cleanup();
    std::string getServiceName();
    virtual bool writeRaw(const std::string &msg);
    virtual bool poll(long timeout = 0);
    virtual bool readRaw(std::string &msg);

protected:
    void resolve();
    void connect();
    void disconnect();

    //! The Node owning this ServiceClient
    Node &node_;

    //! The name of the service
    std::string service_name_;

    //! The ZeroMQ REQ socket
    zmq::socket_t req_socket_;

    //! The address of the ZeroMQ socket in the server
    std::string remote_addr_;

    //! True if this service client is managed (init(), cleanup()) by the Node
    const bool managed_;

    std::string compression_algorithm_;

    int compression_level_;
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
     * \brief Write a request message to the underlying ZeroMQ REQ socket
     */
    virtual bool write(const TReq &req)
    {
        std::string payload;
        return req.SerializeToString(&payload) &&
            AbstractServiceClient::writeRaw(payload);
    }

    /*!
     * \brief Read a reply message from the underlying ZeroMQ REQ socket
     */
    virtual bool read(TRep &rep)
    {
        std::string payload;
        return AbstractServiceClient::readRaw(payload) &&
            rep.ParseFromString(payload);
    }

    /*!
     * \brief Perform initialization and optionally send graph notify
     */
    void init() override
    {
        AbstractServiceClient::init();

        if(notifyGraph)
            b0::graph::notifyService(node_, service_name_, true, true);
    }

    /*!
     * \brief Perform cleanup and optionally send graph notify
     */
    void cleanup() override
    {
        AbstractServiceClient::cleanup();

        if(notifyGraph)
            b0::graph::notifyService(node_, service_name_, true, false);
    }
};

template<>
bool ServiceClient<std::string, std::string, true>::write(const std::string &req);

template<>
bool ServiceClient<std::string, std::string, true>::read(std::string &rep);

} // namespace b0

#endif // B0__SERVICE_CLIENT_H__INCLUDED

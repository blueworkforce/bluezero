#ifndef B0__SERVICE_CLIENT_H__INCLUDED
#define B0__SERVICE_CLIENT_H__INCLUDED

#include <string>

#include <b0/socket.h>

namespace b0
{

class Node;

/*!
 * \brief The service client class
 *
 * This class wraps a REQ socket. It will automatically resolve the address
 * of service name.
 *
 * \sa b0::ServiceClient, b0::ServiceServer
 */
class ServiceClient : public Socket
{
public:
    using logger::LogInterface::log;

    /*!
     * \brief Construct an ServiceClient child of the specified Node
     */
    ServiceClient(Node *node, std::string service_name, bool managed = true, bool notify_graph = true);

    /*!
     * \brief ServiceClient destructor
     */
    virtual ~ServiceClient();

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

    /*!
     * \brief Write a request and read a reply from the underlying ZeroMQ REQ socket
     * \sa ServiceServer::read(), ServiceServer::write()
     */
    virtual void call(const std::string &req, std::string &rep);

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

} // namespace b0

#endif // B0__SERVICE_CLIENT_H__INCLUDED

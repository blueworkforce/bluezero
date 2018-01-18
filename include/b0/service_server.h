#ifndef B0__SERVICE_SERVER_H__INCLUDED
#define B0__SERVICE_SERVER_H__INCLUDED

#include <string>

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <b0/socket.h>

namespace b0
{

class Node;

/*!
 * \brief The service server class
 *
 * This class wraps a REP socket. It will automatically
 * announce the socket name to resolver.
 *
 * \sa b0::ServiceClient, b0::ServiceServer
 */
class ServiceServer : public Socket
{
public:
    using logger::LogInterface::log;

    /*!
     * \brief Construct an ServiceServer child of the specified Node, optionally using a boost::function as a callback
     */
    ServiceServer(Node *node, std::string service_name, boost::function<void(const std::string&, std::string&)> callback = 0, bool managed = true, bool notify_graph = true);

    /*!
     * \brief Construct a ServiceServer child of a specific Node, using a method (of the Node subclass) as callback
     */
    template<class TNode>
    ServiceServer(TNode *node, std::string service_name, void (TNode::*callbackMethod)(const std::string&, std::string&), bool managed = true, bool notify_graph = true)
        : ServiceServer(node, service_name, boost::bind(callbackMethod, node, _1, _2), managed, notify_graph)
    {
        // delegate constructor. leave empty
    }

    /*!
     * \brief Construct a ServiceServer child of a specific Node, using a method as callback
     */
    template<class T>
    ServiceServer(Node *node, std::string service_name, void (T::*callbackMethod)(const std::string&, std::string&), T *callbackObject, bool managed = true, bool notify_graph = true)
        : ServiceServer(node, service_name, boost::bind(callbackMethod, callbackObject, _1, _2), managed, notify_graph)
    {
        // delegate constructor. leave empty
    }

    /*!
     * \brief ServiceServer destructor
     */
    virtual ~ServiceServer();

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
     * \brief Poll and read incoming messages, and dispatch them (called by b0::Node::spinOnce())
     */
    virtual void spinOnce() override;

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

    /*!
     * \brief Callback which will be called when a new message is read from the socket
     */
    boost::function<void(const std::string&, std::string&)> callback_;
};

} // namespace b0

#endif // B0__SERVICE_SERVER_H__INCLUDED

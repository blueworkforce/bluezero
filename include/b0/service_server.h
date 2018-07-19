#ifndef B0__SERVICE_SERVER_H__INCLUDED
#define B0__SERVICE_SERVER_H__INCLUDED

#include <string>

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <b0/socket.h>
#include <b0/message/message_envelope.h>

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

    //! \brief Alias for function
    template<typename T> using function = boost::function<T>;

    //! \brief Alias for callback raw without type
    using CallbackWithoutType = function<void(const std::string&, std::string&)>;

    //! \brief Alias for callback raw with type
    using CallbackWithType = function<void(const std::string&, const std::string&, std::string&, std::string&)>;

    //! \brief Alias for callback raw message parts
    using CallbackRawParts = function<void(const std::vector<b0::message::MessagePart>&, std::vector<b0::message::MessagePart>&)>;

    //! \brief Alias for callback message class
    template<class TReq, class TRep> using CallbackTypedMsg = function<void(const TReq&, TRep&)>;

    /*!
     * \brief Construct an ServiceServer child of the specified Node, without a callback
     */
    ServiceServer(Node *node, std::string service_name, bool managed = true, bool notify_graph = true)
        : ServiceServer(node, service_name, CallbackWithoutType{}, managed, notify_graph)
    {}

    /*!
     * \brief Construct an ServiceServer child of the specified Node, using a function as a callback (raw without type)
     */
    ServiceServer(Node *node, std::string service_name, CallbackWithoutType callback, bool managed = true, bool notify_graph = true);

    /*!
     * \brief Construct an ServiceServer child of the specified Node, using a function as a callback (raw with type)
     */
    ServiceServer(Node *node, std::string service_name, CallbackWithType callback, bool managed = true, bool notify_graph = true);

    /*!
     * \brief Construct an ServiceServer child of the specified Node, using a function as a callback (raw message parts)
     */
    ServiceServer(Node *node, std::string service_name, CallbackRawParts callback, bool managed = true, bool notify_graph = true);

    /*!
     * \brief Construct an ServiceServer child of the specified Node, using a function as a callback (message class)
     */
    template<class TReq, class TRep>
    ServiceServer(Node *node, std::string service_name, CallbackTypedMsg<TReq, TRep> callback, bool managed = true, bool notify_graph = true)
        : ServiceServer(node, service_name,
                static_cast<CallbackRawParts>([&, callback](const std::vector<b0::message::MessagePart> &reqparts, std::vector<b0::message::MessagePart> &repparts) {
                    TReq req; TRep rep;
                    req.parseFromString(reqparts[0].payload);
                    callback(req, rep);
                    repparts.resize(1);
                    rep.serializeToString(repparts[0].payload);
                    repparts[0].content_type = rep.type();
                }), managed, notify_graph)
    {}

    /*!
     * \brief Construct an ServiceServer child of the specified Node, using a function as a callback (raw without type, function ptr)
     */
    ServiceServer(Node *node, std::string service_name, void (*callback)(const std::string&, std::string&), bool managed = true, bool notify_graph = true)
        : ServiceServer(node, service_name, static_cast<CallbackWithoutType>(callback), managed, notify_graph)
    {}

    /*!
     * \brief Construct an ServiceServer child of the specified Node, using a function as a callback (raw with type, function ptr)
     */
    ServiceServer(Node *node, std::string service_name, void (*callback)(const std::string&, const std::string&, std::string&, std::string&), bool managed = true, bool notify_graph = true)
        : ServiceServer(node, service_name, static_cast<CallbackWithType>(callback), managed, notify_graph)
    {}

    /*!
     * \brief Construct an ServiceServer child of the specified Node, using a function as a callback (raw message parts, function ptr)
     */
    ServiceServer(Node *node, std::string service_name, void (*callback)(const std::vector<b0::message::MessagePart>&, std::vector<b0::message::MessagePart>&), bool managed = true, bool notify_graph = true)
        : ServiceServer(node, service_name, static_cast<CallbackRawParts>(callback), managed, notify_graph)
    {}

    /*!
     * \brief Construct an ServiceServer child of the specified Node, using a function as a callback (message class, function ptr)
     */
    template<class TReq, class TRep>
    ServiceServer(Node *node, std::string service_name, void (*callback)(const TReq&, TRep&), bool managed = true, bool notify_graph = true)
        : ServiceServer(node, service_name, static_cast<CallbackTypedMsg<TReq, TRep> >(callback), managed, notify_graph)
    {}

    /*!
     * \brief Construct an ServiceServer child of the specified Node, using a function as a callback (raw without type, method ptr)
     */
    template<class T>
    ServiceServer(Node *node, std::string service_name, void (T::*callback)(const std::string&, std::string&), T *obj, bool managed = true, bool notify_graph = true)
        : ServiceServer(node, service_name, static_cast<CallbackWithoutType>(boost::bind(callback, obj, _1, _2)), managed, notify_graph)
    {}

    /*!
     * \brief Construct an ServiceServer child of the specified Node, using a function as a callback (raw with type, method ptr)
     */
    template<class T>
    ServiceServer(Node *node, std::string service_name, void (T::*callback)(const std::string&, const std::string&, std::string&, std::string&), T *obj, bool managed = true, bool notify_graph = true)
        : ServiceServer(node, service_name, static_cast<CallbackWithType>(boost::bind(callback, obj, _1, _2, _3, _4)), managed, notify_graph)
    {}

    /*!
     * \brief Construct an ServiceServer child of the specified Node, using a function as a callback (raw message parts, method ptr)
     */
    template<class T>
    ServiceServer(Node *node, std::string service_name, void (T::*callback)(const std::vector<b0::message::MessagePart>&, std::vector<b0::message::MessagePart>&), T *obj, bool managed = true, bool notify_graph = true)
        : ServiceServer(node, service_name, static_cast<CallbackRawParts>(boost::bind(callback, obj, _1, _2)), managed, notify_graph)
    {}

    /*!
     * \brief Construct an ServiceServer child of the specified Node, using a function as a callback (message class, method ptr)
     */
    template<class T, class TReq, class TRep>
    ServiceServer(Node *node, std::string service_name, void (T::*callback)(const TReq&, TRep&), T *obj, bool managed = true, bool notify_graph = true)
        : ServiceServer(node, service_name, static_cast<CallbackTypedMsg<TReq, TRep> >(boost::bind(callback, obj, _1, _2)), managed, notify_graph)
    {}

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
     * \brief Callback which will be called when a new message is read from the socket (raw)
     */
    CallbackWithoutType callback_;

    /*!
     * \brief Callback which will be called when a new message is read from the socket (raw with type)
     */
    CallbackWithType callback_with_type_;

    /*!
     * \brief Callback which will be called when a new message is read from the socket (raw multipart)
     */
    CallbackRawParts callback_multipart_;
};

} // namespace b0

#endif // B0__SERVICE_SERVER_H__INCLUDED

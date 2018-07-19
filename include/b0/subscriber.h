#ifndef B0__SUBSCRIBER_H__INCLUDED
#define B0__SUBSCRIBER_H__INCLUDED

#include <string>

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <b0/socket.h>
#include <b0/message/message_envelope.h>

namespace b0
{

class Node;

/*!
 * \brief The subscriber class
 *
 * This class wraps a SUB socket. It will automatically connect to the
 * XPUB socket of the proxy (note: the proxy is started by the resolver node).
 *
 * In order to receive some message, you must set a topic subscription with Subscriber::subscribe.
 * You can set multiple subscription, and the incoming messages will match any of those.
 *
 * The subscription topics are strings and are matched on a prefix basis.
 * (i.e. if the topic of the incoming message is "AAA", a subscription for "A" will match it)
 *
 * \sa b0::Publisher, b0::Subscriber
 */
class Subscriber : public Socket
{
public:
    using logger::LogInterface::log;

    //! \brief Alias for function
    template<typename T> using function = boost::function<T>;

    //! \brief Alias for callback raw without type
    using CallbackWithoutType = function<void(const std::string&)>;

    //! \brief Alias for callback raw with type
    using CallbackWithType = function<void(const std::string&, const std::string&)>;

    //! \brief Alias for callback raw message parts
    using CallbackRawParts = function<void(const std::vector<b0::message::MessagePart>&)>;

    //! \brief Alias for callback message class
    template<class TMsg> using CallbackTypedMsg = function<void(const TMsg&)>;

    /*!
     * \brief Construct an Subscriber child of the specified Node without a callback
     */
    Subscriber(Node *node, std::string topic_name, bool managed = true, bool notify_graph = true)
        : Subscriber(node, topic_name, CallbackWithoutType{}, managed, notify_graph)
    {}

    /*!
     * \brief Construct an Subscriber child of the specified Node, optionally using a function as callback (raw without type)
     */
    Subscriber(Node *node, std::string topic_name, CallbackWithoutType callback, bool managed = true, bool notify_graph = true);

    /*!
     * \brief Construct an Subscriber child of the specified Node, using a function as callback (raw with type)
     */
    Subscriber(Node *node, std::string topic_name, CallbackWithType callback, bool managed = true, bool notify_graph = true);

    /*!
     * \brief Construct an Subscriber child of the specified Node, using a function as callback (raw message parts)
     */
    Subscriber(Node *node, std::string topic_name, CallbackRawParts callback, bool managed = true, bool notify_graph = true);

    /*!
     * \brief Construct an Subscriber child of the specified Node, using a function as callback (message class)
     */
    template<class TMsg>
    Subscriber(Node *node, std::string service_name, CallbackTypedMsg<TMsg> callback, bool managed = true, bool notify_graph = true)
        : Subscriber(node, service_name,
                static_cast<CallbackRawParts>([&, callback](const std::vector<b0::message::MessagePart> &parts) {
                    TMsg msg;
                    msg.parseFromString(parts[0].payload);
                    callback(msg);
                }), managed, notify_graph)
    {}

    /*!
     * \brief Construct an Subscriber child of the specified Node, using a function as callback (raw without type, function ptr)
     */
    Subscriber(Node *node, std::string service_name, void (*callback)(const std::string&), bool managed = true, bool notify_graph = true)
        : Subscriber(node, service_name, static_cast<CallbackWithoutType>(callback), managed, notify_graph)
    {}

    /*!
     * \brief Construct an Subscriber child of the specified Node, using a function as callback (raw with type, function ptr)
     */
    Subscriber(Node *node, std::string service_name, void (*callback)(const std::string&, const std::string&), bool managed = true, bool notify_graph = true)
        : Subscriber(node, service_name, static_cast<CallbackWithType>(callback), managed, notify_graph)
    {}

    /*!
     * \brief Construct an Subscriber child of the specified Node, using a function as callback (raw message parts, function ptr)
     */
    Subscriber(Node *node, std::string service_name, void (*callback)(const std::vector<b0::message::MessagePart>&), bool managed = true, bool notify_graph = true)
        : Subscriber(node, service_name, static_cast<CallbackRawParts>(callback), managed, notify_graph)
    {}

    /*!
     * \brief Construct an Subscriber child of the specified Node, using a function as callback (message class, function ptr)
     */
    template<class TMsg>
    Subscriber(Node *node, std::string service_name, void (*callback)(const TMsg&), bool managed = true, bool notify_graph = true)
        : Subscriber(node, service_name, static_cast<CallbackTypedMsg<TMsg> >(callback), managed, notify_graph)
    {}

    /*!
     * \brief Construct an Subscriber child of the specified Node, using a function as callback (raw without type, method ptr)
     */
    template<class T>
    Subscriber(Node *node, std::string service_name, void (T::*callback)(const std::string&), T *obj, bool managed = true, bool notify_graph = true)
        : Subscriber(node, service_name, static_cast<CallbackWithoutType>(boost::bind(callback, obj, _1)))
    {}

    /*!
     * \brief Construct an Subscriber child of the specified Node, using a function as callback (raw with type, method ptr)
     */
    template<class T>
    Subscriber(Node *node, std::string service_name, void (T::*callback)(const std::string&, const std::string&), T *obj, bool managed = true, bool notify_graph = true)
        : Subscriber(node, service_name, static_cast<CallbackWithType>(boost::bind(callback, obj, _1, _2)))
    {}

    /*!
     * \brief Construct an Subscriber child of the specified Node, using a function as callback (raw message parts, method ptr)
     */
    template<class T>
    Subscriber(Node *node, std::string service_name, void (T::*callback)(const std::vector<b0::message::MessagePart>&), T *obj, bool managed = true, bool notify_graph = true)
        : Subscriber(node, service_name, static_cast<CallbackWithType>(boost::bind(callback, obj, _1, _2)))
    {}

    /*!
     * \brief Construct an Subscriber child of the specified Node, using a function as callback (message class, method ptr)
     */
    template<class T, class TMsg>
    Subscriber(Node *node, std::string service_name, void (T::*callback)(const TMsg&), T *obj, bool managed = true, bool notify_graph = true)
        : Subscriber(node, service_name, static_cast<CallbackTypedMsg<TMsg> >(boost::bind(callback, obj, _1)))
    {}

    /*!
     * \brief Subscriber destructor
     */
    virtual ~Subscriber();

    /*!
     * \brief Log a message using node's logger, prepending this subscriber informations
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
     * \brief Process incoming messages and call callbacks
     */
    virtual void spinOnce() override;

    /*!
     * \brief Return the name of this subscriber's topic
     */
    std::string getTopicName();

protected:
    /*!
     * \brief Connect to the remote address
     */
    virtual void connect();

    /*!
     * \brief Disconnect from the remote address
     */
    virtual void disconnect();

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

#endif // B0__SUBSCRIBER_H__INCLUDED

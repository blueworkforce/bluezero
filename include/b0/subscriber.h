#ifndef B0__SUBSCRIBER_H__INCLUDED
#define B0__SUBSCRIBER_H__INCLUDED

#include <string>

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <b0/socket/socket.h>

namespace b0
{

class Node;

/*!
 * \brief The (abstract) subscriber class
 *
 * This class wraps a ZeroMQ SUB socket. It will automatically connect to the
 * XPUB socket of the proxy (note: the proxy is started by the resolver node).
 *
 * In order to receive some message, you must set a topic subscription with Subscriber::subscribe.
 * You can set multiple subscription, and the incoming messages will match any of those.
 *
 * The subscription topics are strings and are matched on a prefix basis.
 * (i.e. if the topic of the incoming message is "AAA", a subscription for "A" will match it)
 *
 * \sa b0::Publisher, b0::Subscriber, b0::AbstractPublisher, b0::AbstractSubscriber
 */
class AbstractSubscriber : public socket::Socket
{
public:
    using logger::LogInterface::log;

    /*!
     * \brief Construct an AbstractSubscriber child of the specified Node
     */
    AbstractSubscriber(Node *node, std::string topic_name, bool managed = true, bool notify_graph = true);

    /*!
     * \brief AbstractSubscriber destructor
     */
    virtual ~AbstractSubscriber();

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
};

/*!
 * \brief The subscriber template class
 *
 * This template class specializes b0::AbstractSubscriber to a specific message type,
 * and it implements the spinOnce method as well.
 *
 * Important when using a callback: you must call b0::Node::spin(), or periodically call
 * b0::Node::spinOnce(), otherwise no message will be delivered.
 *
 * Otherwise, you can directly read from the SUB socket, by using Subscriber::read().
 * Note: read operation is blocking. If you do not want to block, use Subscriber::poll() first.
 *
 * \sa b0::Publisher, b0::Subscriber, b0::AbstractPublisher, b0::AbstractSubscriber
 */
template<typename TMsg>
class Subscriber : public AbstractSubscriber
{
public:
    /*!
     * \brief Construct a Subscriber child of a specified Node, with a boost::function as callback
     */
    Subscriber(Node *node, std::string topic_name, boost::function<void(const TMsg&)> callback = 0, bool managed = true, bool notify_graph = true)
        : AbstractSubscriber(node, topic_name, managed, notify_graph),
          callback_(callback)
    {
    }

    /*!
     * \brief Construct a Subscriber child of a specified Node, with a method (of the Node subclass) as a callback
     */
    template<class TNode>
    Subscriber(TNode *node, std::string topic_name, void (TNode::*callbackMethod)(const TMsg&), bool managed = true, bool notify_graph = true)
        : Subscriber(node, topic_name, boost::bind(callbackMethod, node, _1), managed, notify_graph)
    {
        // delegate constructor. leave empty
    }

    /*!
     * \brief Construct a Subscriber child of a specified Node, with a method as a callback
     */
    template<class T>
    Subscriber(Node *node, std::string topic_name, void (T::*callbackMethod)(const TMsg&), T *callbackObject, bool managed = true, bool notify_graph = true)
        : Subscriber(node, topic_name, boost::bind(callbackMethod, callbackObject, _1), managed, notify_graph)
    {
        // delegate constructor. leave empty
    }

    /*!
     * \brief Poll and read incoming messages, and dispatch them (called by b0::Node::spinOnce())
     */
    void spinOnce() override
    {
        if(callback_.empty()) return;

        while(poll())
        {
            TMsg msg;
            if(read(msg))
                callback_(msg);
        }
    }
 
protected:
    /*!
     * \brief Callback which will be called when a new message is read from the socket
     */
    boost::function<void(TMsg&)> callback_;
};

} // namespace b0

#endif // B0__SUBSCRIBER_H__INCLUDED

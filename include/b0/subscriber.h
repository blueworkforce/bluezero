#ifndef B0__SUBSCRIBER_H__INCLUDED
#define B0__SUBSCRIBER_H__INCLUDED

#include <string>

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <b0/socket/socket.h>
#include <b0/graph/graph.h>

namespace b0
{

class Node;

//! \cond HIDDEN_SYMBOLS

class AbstractSubscriber : public socket::Socket
{
public:
    using logger::LogInterface::log;

    AbstractSubscriber(Node *node, std::string topic_name, bool managed = true);
    virtual ~AbstractSubscriber();
    void log(LogLevel level, std::string message) const override;
    virtual void init() override;
    virtual void cleanup() override;
    std::string getTopicName();

protected:
    virtual void connect();
    virtual void disconnect();
};

//! \endcond

/*!
 * \brief The subscriber class
 *
 * This class wraps a ZeroMQ SUB socket. It will automatically connect to the
 * XPUB socket of the proxy (note: the proxy is started by the resolver node).
 *
 * Important when using a callback: you must call b0::Node::spin(), or periodically call
 * b0::Node::spinOnce(), otherwise no message will be delivered.
 *
 * Otherwise, you can directly read from the SUB socket, by using Subscriber::read().
 * Note: read operation is blocking. If you do not want to block, use Subscriber::poll() first.
 *
 * In order to receive some message, you must set a topic subscription with Subscriber::subscribe.
 * You can set multiple subscription, and the incoming messages will match any of those.
 *
 * The subscription topics are strings and are matched on a prefix basis.
 * (i.e. if the topic of the incoming message is "AAA", a subscription for "A" will match it)
 *
 * \sa b0::Publisher
 */
template<typename TMsg, bool notifyGraph = true>
class Subscriber : public AbstractSubscriber
{
public:
    /*!
     * \brief Construct a Subscriber child of a specified Node, with a boost::function as callback
     */
    Subscriber(Node *node, std::string topic_name, boost::function<void(const TMsg&)> callback = 0, bool managed = true)
        : AbstractSubscriber(node, topic_name, managed),
          callback_(callback)
    {
    }

    /*!
     * \brief Construct a Subscriber child of a specified Node, with a method (of the Node subclass) as a callback
     */
    template<class TNode>
    Subscriber(TNode *node, std::string topic_name, void (TNode::*callbackMethod)(const TMsg&), bool managed = true)
        : Subscriber(node, topic_name, boost::bind(callbackMethod, node, _1), managed)
    {
        // delegate constructor. leave empty
    }

    /*!
     * \brief Construct a Subscriber child of a specified Node, with a method as a callback
     */
    template<class T>
    Subscriber(Node *node, std::string topic_name, void (T::*callbackMethod)(const TMsg&), T *callbackObject, bool managed = true)
        : Subscriber(node, topic_name, boost::bind(callbackMethod, callbackObject, _1), managed)
    {
        // delegate constructor. leave empty
    }

    /*!
     * \brief Perform initialization and optionally send graph notify
     */
    void init() override
    {
        AbstractSubscriber::init();

        if(notifyGraph)
            b0::graph::notifyTopic(node_, name_, true, true);
    }

    /*!
     * \brief Perform cleanup and optionally send graph notify
     */
    void cleanup() override
    {
        AbstractSubscriber::cleanup();

        if(notifyGraph)
            b0::graph::notifyTopic(node_, name_, true, false);
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

#ifndef B0__SUBSCRIBER_H__INCLUDED
#define B0__SUBSCRIBER_H__INCLUDED

#include <string>

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <b0/utils/protobufhelpers.h>
#include <b0/graph/graph.h>

namespace b0
{

class Node;

//! \cond HIDDEN_SYMBOLS

class AbstractSubscriber
{
public:
    AbstractSubscriber(Node *node, std::string topic, bool managed = true);
    virtual ~AbstractSubscriber();
    void setRemoteAddress(std::string addr);
    virtual void init();
    virtual void cleanup();
    virtual void spinOnce() = 0;
    std::string getTopicName();
    virtual bool poll(long timeout = 0);
    virtual bool readRaw(std::string &topic, std::string &msg);

protected:
    //! The Node owning this Subscriber
    Node &node_;

    //! The ZeroMQ SUB socket
    zmq::socket_t sub_socket_;

    //! The ZeroMQ topic name to set the subscription
    std::string topic_name_;

    //! True if this subscriber is managed (init(), cleanup() are called by the owner Node)
    const bool managed_;

    //! If set, this publisher will connect directly to the given address instead of the XSUB address given by the owner Node
    //! \sa AbstractSubscriber::setRemoteAddress()
    std::string remote_addr_;
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
    Subscriber(Node *node, std::string topic, boost::function<void(std::string, const TMsg&)> callback = 0, bool managed = true)
        : AbstractSubscriber(node, topic, managed),
          callback_(callback)
    {
    }

    /*!
     * \brief Construct a Subscriber child of a specified Node, with a method (of the Node subclass) as a callback
     */
    template<class TNode>
    Subscriber(TNode *node, std::string topic, void (TNode::*callbackMethod)(std::string, const TMsg&), bool managed = true)
        : Subscriber(node, topic, boost::bind(callbackMethod, node, _1, _2), managed)
    {
        // delegate constructor. leave empty
    }

    /*!
     * \brief Construct a Subscriber child of a specified Node, with a method as a callback
     */
    template<class T>
    Subscriber(Node *node, std::string topic, void (T::*callbackMethod)(std::string, const TMsg&), T *callbackObject, bool managed = true)
        : Subscriber(node, topic, boost::bind(callbackMethod, callbackObject, _1, _2), managed)
    {
        // delegate constructor. leave empty
    }

    /*!
     * \brief Poll and read incoming messages, and dispatch them (called by b0::Node::spinOnce())
     */
    virtual void spinOnce() override
    {
        if(callback_.empty()) return;

        while(poll())
        {
            std::string topic;
            TMsg msg;
            read(topic, msg);
            callback_(topic, msg);
        }
    }

    /*!
     * \brief Read a message from the underlying ZeroMQ SUB socket
     */
    virtual bool read(std::string &topic, TMsg &msg)
    {
        std::string payload;
        return AbstractSubscriber::readRaw(topic, payload) &&
            msg.ParseFromString(payload);
    }

    /*!
     * \brief Perform initialization and optionally send graph notify
     */
    void init() override
    {
        AbstractSubscriber::init();

        if(notifyGraph)
            b0::graph::notifyTopic(node_, topic_name_, true, true);
    }

    /*!
     * \brief Perform cleanup and optionally send graph notify
     */
    void cleanup() override
    {
        AbstractSubscriber::cleanup();

        if(notifyGraph)
            b0::graph::notifyTopic(node_, topic_name_, true, false);
    }

protected:
    /*!
     * \brief Callback which will be called when a new message is read from the socket
     */
    boost::function<void(std::string, TMsg&)> callback_;
};

/*!
 * \brief Read a raw payload from the underlying ZeroMQ SUB socket
 */
template<>
bool Subscriber<std::string, true>::read(std::string &topic, std::string &msg);

} // namespace b0

#endif // B0__SUBSCRIBER_H__INCLUDED

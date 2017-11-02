#ifndef SUBSCRIBER_H_INCLUDED
#define SUBSCRIBER_H_INCLUDED

#include <b0/node.h>
#include <b0/graph/graph.h>

namespace b0
{

//! \cond HIDDEN_SYMBOLS

class AbstractSubscriber
{
public:
    AbstractSubscriber(Node *node, std::string topic);
    virtual ~AbstractSubscriber();
    virtual void init();
    virtual void cleanup();
    virtual void spinOnce() = 0;
    std::string getTopicName();

protected:
    //! The Node owning this Subscriber
    Node &node_;

    //! The ZeroMQ SUB socket
    zmq::socket_t sub_socket_;

    //! The ZeroMQ topic name to set the subscription
    std::string topic_name_;
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
    Subscriber(Node *node, std::string topic, boost::function<void(std::string, const TMsg&)> callback = 0)
        : AbstractSubscriber(node, topic),
          callback_(callback)
    {
    }

    /*!
     * \brief Construct a Subscriber child of a specified Node, with a method (of the Node subclass) as a callback
     */
    template<class TNode>
    Subscriber(TNode *node, std::string topic, void (TNode::*callbackMethod)(std::string, const TMsg&))
        : Subscriber(node, topic, boost::bind(callbackMethod, node, _1, _2))
    {
        // delegate constructor. leave empty
    }

    /*!
     * \brief Construct a Subscriber child of a specified Node, with a method as a callback
     */
    template<class T>
    Subscriber(Node *node, std::string topic, void (T::*callbackMethod)(std::string, const TMsg&), T *callbackObject)
        : Subscriber(node, topic, boost::bind(callbackMethod, callbackObject, _1, _2))
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
     * \brief Poll the underlying ZeroMQ SUB socket
     */
    virtual bool poll(long timeout = 0)
    {
#ifdef __GNUC__
        zmq::pollitem_t items[] = {{static_cast<void*>(sub_socket_), 0, ZMQ_POLLIN, 0}};
#else
        zmq::pollitem_t items[] = {{sub_socket_, 0, ZMQ_POLLIN, 0}};
#endif
        zmq::poll(&items[0], sizeof(items) / sizeof(items[0]), timeout);
        return items[0].revents & ZMQ_POLLIN;
    }

    /*!
     * \brief Read a message from the underlying ZeroMQ SUB socket
     */
    virtual bool read(std::string &topic, TMsg &msg)
    {
        topic = ::s_recv(sub_socket_);
        bool ret = ::s_recv(sub_socket_, msg);
        return ret;
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
        AbstractSubscriber::init();

        if(notifyGraph)
            b0::graph::notifyTopic(node_, topic_name_, true, false);
    }

protected:
    /*!
     * \brief Callback which will be called when a new message is read from the socket
     */
    boost::function<void(std::string, TMsg&)> callback_;
};

template<>
bool Subscriber<std::string, true>::read(std::string &topic, std::string &msg);

} // namespace b0

#endif // SUBSCRIBER_H_INCLUDED

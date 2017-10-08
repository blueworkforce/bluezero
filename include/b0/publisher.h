#ifndef PUBLISHER_H_INCLUDED
#define PUBLISHER_H_INCLUDED

#include <b0/node.h>
#include <b0/graph.h>

namespace b0
{

//! \cond HIDDEN_SYMBOLS

class AbstractPublisher
{
public:
    AbstractPublisher(Node *node, std::string topic)
        : node_(*node),
          pub_socket_(node_.getZMQContext(), ZMQ_PUB),
          topic_(topic)
    {
        node_.addPublisher(this);
    }

    virtual ~AbstractPublisher()
    {
        node_.removePublisher(this);
    }

    /*!
     * \brief Perform initialization (connect to XSUB)
     */
    virtual void init()
    {
        pub_socket_.connect(node_.getXSUBSocketAddress());
    }

protected:
    //! The Node owning this Publisher
    Node &node_;

    //! The ZeroMQ PUB socket
    zmq::socket_t pub_socket_;

    //! The ZeroMQ envelope address (also known as the topic)
    std::string topic_;
};

//! \endcond

/*!
 * \brief The publisher class
 *
 * This class wraps a ZeroMQ PUB socket. It will automatically connect to the
 * XSUB socket of the proxy (note: the proxy is started by the resolver node).
 *
 * \sa b0::Subscriber
 */
template<typename TMsg, bool notifyGraph = true>
class Publisher : public AbstractPublisher
{
public:
    /*!
     * \brief Construct a Publisher child of the specified Node
     */
    Publisher(Node *node, std::string topic)
        : AbstractPublisher(node, topic)
    {
    }

    /*!
     * \brief Publish the message on the publisher's topic
     */
    virtual void publish(const TMsg &msg)
    {
        ::s_sendmore(pub_socket_, topic_);
        ::s_send(pub_socket_, msg);
    }

    void init() override
    {
        AbstractPublisher::init();

        if(notifyGraph)
            b0::graph::notifyTopic(node_, topic_, false, true);
    }
};

} // namespace b0

#endif // PUBLISHER_H_INCLUDED

#ifndef PUBLISHER_H_INCLUDED
#define PUBLISHER_H_INCLUDED

#include <string>

#include <b0/utils/protobufhelpers.hpp>
#include <b0/graph/graph.h>

namespace b0
{

class Node;

//! \cond HIDDEN_SYMBOLS

class AbstractPublisher
{
public:
    AbstractPublisher(Node *node, std::string topic, bool managed = true);
    virtual ~AbstractPublisher();
    void setRemoteAddress(std::string addr);
    virtual void init();
    virtual void cleanup();
    std::string getTopicName();
    virtual bool writeRaw(const std::string &topic, const std::string &msg);

protected:
    //! The Node owning this Publisher
    Node &node_;

    //! The ZeroMQ PUB socket
    zmq::socket_t pub_socket_;

    //! The ZeroMQ envelope address (also known as the topic)
    std::string topic_name_;

    //! True if this publisher is managed (init(), cleanup()) by the Node 
    const bool managed_;

    std::string remote_addr_;
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
    Publisher(Node *node, std::string topic, bool managed = true)
        : AbstractPublisher(node, topic, managed)
    {
    }

    /*!
     * \brief Publish the message on the specified topic (don't use this, use Publisher::publish() instead)
     */
    virtual bool write(const std::string &topic, const TMsg &msg)
    {
        std::string payload;
        return msg.SerializeToString(&payload) &&
            AbstractPublisher::writeRaw(topic_name_, payload);
    }

    /*!
     * \brief Publish the message on the publisher's topic
     */
    virtual void publish(const TMsg &msg)
    {
        write(topic_name_, msg);
    }

    /*!
     * \brief Perform initialization and optionally send graph notify
     */
    void init() override
    {
        AbstractPublisher::init();

        if(notifyGraph)
            b0::graph::notifyTopic(node_, topic_name_, false, true);
    }

    /*!
     * \brief Perform cleanup and optionally send graph notify
     */
    void cleanup() override
    {
        AbstractPublisher::cleanup();

        if(notifyGraph)
            b0::graph::notifyTopic(node_, topic_name_, false, false);
    }
};

template<>
bool Publisher<std::string, true>::write(const std::string &topic, const std::string &msg);

} // namespace b0

#endif // PUBLISHER_H_INCLUDED

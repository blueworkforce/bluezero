#ifndef B0__PUBLISHER_H__INCLUDED
#define B0__PUBLISHER_H__INCLUDED

#include <string>

#include <b0/utils/protobufhelpers.h>
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
    void setCompression(std::string algorithm, int level = -1);
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

    //! True if this publisher is managed (init(), cleanup() are called by the owner Node)
    const bool managed_;

    //! If set, this publisher will connect directly to the given address instead of the XSUB address given by the owner Node
    //! \sa AbstractPublisher::setRemoteAddress()
    std::string remote_addr_;

    //! If set, payloads will be encoded using the specified compression algorithm
    //! \sa AbstractPublisher::setCompression()
    std::string compression_algorithm_;

    //! If a compression algorithm is set, payloads will be encoded using the specified compression level
    //! \sa AbstractPublisher::setCompression()
    int compression_level_;
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

/*!
 * \brief Publish a raw payload on the specified topic
 */
template<>
bool Publisher<std::string, true>::write(const std::string &topic, const std::string &msg);

} // namespace b0

#endif // B0__PUBLISHER_H__INCLUDED

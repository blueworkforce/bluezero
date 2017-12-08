#ifndef B0__PUBLISHER_H__INCLUDED
#define B0__PUBLISHER_H__INCLUDED

#include <string>

#include <b0/socket/socket.h>
#include <b0/graph/graph.h>

namespace b0
{

class Node;

//! \cond HIDDEN_SYMBOLS

class AbstractPublisher : public socket::Socket
{
public:
    using logger::LogInterface::log;

    AbstractPublisher(Node *node, std::string topic, bool managed = true);
    virtual ~AbstractPublisher();
    void log(LogLevel level, std::string message) const override;
    virtual void init() override;
    virtual void cleanup() override;
    virtual void spinOnce() override {}
    std::string getTopicName();

protected:
    virtual void connect();
    virtual void disconnect();
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
    Publisher(Node *node, std::string topic_name, bool managed = true)
        : AbstractPublisher(node, topic_name, managed)
    {
    }

    /*!
     * \brief Publish the message on the publisher's topic
     */
    virtual void publish(const TMsg &msg)
    {
        write(msg);
    }

    /*!
     * \brief Perform initialization and optionally send graph notify
     */
    void init() override
    {
        AbstractPublisher::init();

        if(notifyGraph)
            b0::graph::notifyTopic(node_, name_, false, true);
    }

    /*!
     * \brief Perform cleanup and optionally send graph notify
     */
    void cleanup() override
    {
        AbstractPublisher::cleanup();

        if(notifyGraph)
            b0::graph::notifyTopic(node_, name_, false, false);
    }
};

} // namespace b0

#endif // B0__PUBLISHER_H__INCLUDED

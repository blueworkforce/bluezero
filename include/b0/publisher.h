#ifndef B0__PUBLISHER_H__INCLUDED
#define B0__PUBLISHER_H__INCLUDED

#include <string>

#include <b0/socket/socket.h>

namespace b0
{

class Node;

//! \cond HIDDEN_SYMBOLS

class AbstractPublisher : public socket::Socket
{
public:
    using logger::LogInterface::log;

    AbstractPublisher(Node *node, std::string topic, bool managed, bool notify_graph);

    virtual ~AbstractPublisher();

    void log(LogLevel level, std::string message) const override;

    /*!
     * \brief Perform initialization and optionally send graph notify
     */
    virtual void init() override;

    /*!
     * \brief Perform cleanup and optionally send graph notify
     */
    virtual void cleanup() override;

    virtual void spinOnce() override {}

    std::string getTopicName();

protected:
    virtual void connect();
    virtual void disconnect();

    const bool notify_graph_;
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
template<typename TMsg>
class Publisher : public AbstractPublisher
{
public:
    /*!
     * \brief Construct a Publisher child of the specified Node
     */
    Publisher(Node *node, std::string topic_name, bool managed = true, bool notify_graph = true)
        : AbstractPublisher(node, topic_name, managed, notify_graph)
    {
    }

    /*!
     * \brief Publish the message on the publisher's topic
     */
    virtual void publish(const TMsg &msg)
    {
        write(msg);
    }
};

} // namespace b0

#endif // B0__PUBLISHER_H__INCLUDED

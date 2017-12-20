#ifndef B0__PUBLISHER_H__INCLUDED
#define B0__PUBLISHER_H__INCLUDED

#include <string>

#include <b0/socket/socket.h>

namespace b0
{

class Node;

/*!
 * \brief The (abstract) publisher class
 *
 * This class wraps a ZeroMQ PUB socket. It will automatically connect to the
 * XSUB socket of the proxy (note: the proxy is started by the resolver node).
 *
 * \sa b0::Publisher, b0::Subscriber, b0::AbstractPublisher, b0::AbstractSubscriber
 */
class AbstractPublisher : public socket::Socket
{
public:
    using logger::LogInterface::log;

    /*!
     * \brief Construct an AbstractPublisher child of the specified Node
     */
    AbstractPublisher(Node *node, std::string topic, bool managed = true, bool notify_graph = true);

    /*!
     * \brief AbstractPublisher destructor
     */
    virtual ~AbstractPublisher();

    /*!
     * \brief Log a message using node's logger, prepending this publisher informations
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
     * \brief Return the name of this publisher's topic
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
 * \brief The publisher template class
 *
 * This template class specializes b0::AbstractPublisher to a specific message type.
 *
 * \sa b0::Publisher, b0::Subscriber, b0::AbstractPublisher, b0::AbstractSubscriber
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

template<>
inline void Publisher<std::string>::publish(const std::string &msg)
{
    writeRaw(msg);
}

} // namespace b0

#endif // B0__PUBLISHER_H__INCLUDED

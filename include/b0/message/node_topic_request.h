#ifndef B0__MESSAGE__NODE_TOPIC_REQUEST_H__INCLUDED
#define B0__MESSAGE__NODE_TOPIC_REQUEST_H__INCLUDED

#include <boost/serialization/string.hpp>

#include <b0/message/message.h>

namespace b0
{

namespace message
{

/*!
 * \brief Sent by node to tell a topic it is publishing onto/subscribing to
 *
 * \mscfile graph-topic.msc
 *
 * \sa NodeTopicResponse, \ref protocol, \ref graph
 */
class NodeTopicRequest : public Message
{
public:
    //! The name of the node
    std::string node_name;

    //! The name of the topic
    std::string topic_name;

    //! If true, node is a subscriber, otherwise a publisher
    bool reverse;

    //! If true, the relationship is starting, otherwise is ending
    bool active;

public:
    std::string type() const override;

private:
    void serialize(serialization::MessageFields &fields) const override;
};

} // namespace message

} // namespace b0

#endif // B0__MESSAGE__NODE_TOPIC_REQUEST_H__INCLUDED

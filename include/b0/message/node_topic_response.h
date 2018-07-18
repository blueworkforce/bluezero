#ifndef B0__MESSAGE__NODE_TOPIC_RESPONSE_H__INCLUDED
#define B0__MESSAGE__NODE_TOPIC_RESPONSE_H__INCLUDED

#include <b0/message/message.h>

namespace b0
{

namespace message
{

/*!
 * \brief Response to NodeTopicRequest message
 *
 * \mscfile graph-topic.msc
 *
 * \sa NodeTopicRequest, \ref protocol, \ref graph
 */
class NodeTopicResponse : public Message
{
public:

public:
    std::string type() const override;

private:
    void serialize(serialization::MessageFields &fields) const override;
};

} // namespace message

} // namespace b0

#endif // B0__MESSAGE__NODE_TOPIC_RESPONSE_H__INCLUDED

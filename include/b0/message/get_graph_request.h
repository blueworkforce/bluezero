#ifndef B0__MESSAGE__GET_GRAPH_REQUEST_H__INCLUDED
#define B0__MESSAGE__GET_GRAPH_REQUEST_H__INCLUDED

#include <b0/message/message.h>

namespace b0
{

namespace message
{

/*!
 * \brief Sent by node to resolver, for getting the full graph
 *
 * \mscfile graph-get.msc
 *
 * \sa GetGraphResponse, \ref protocol, \ref graph
 */
class GetGraphRequest : public Message
{
public:

public:
    std::string type() const override;

private:
    void serialize(serialization::MessageFields &fields) const override;
};

} // namespace message

} // namespace b0

#endif // B0__MESSAGE__GET_GRAPH_REQUEST_H__INCLUDED

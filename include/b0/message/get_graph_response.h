#ifndef B0__MESSAGE__GET_GRAPH_RESPONSE_H__INCLUDED
#define B0__MESSAGE__GET_GRAPH_RESPONSE_H__INCLUDED

#include <b0/message/message.h>
#include <b0/message/graph.h>

namespace b0
{

namespace message
{

/*!
 * \brief Response to GetGraphRequest message
 *
 * \mscfile graph-get.msc
 *
 * \sa GetGraphRequest, \ref protocol, \ref graph
 */
class GetGraphResponse : public Message
{
public:
    //! The graph of the network
    Graph graph;

public:
    std::string type() const override;

private:
    void serialize(serialization::MessageFields &fields) const override;
};

} // namespace message

} // namespace b0

#endif // B0__MESSAGE__GET_GRAPH_RESPONSE_H__INCLUDED

#ifndef B0__MESSAGE__GRAPH_H__INCLUDED
#define B0__MESSAGE__GRAPH_H__INCLUDED

#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include <b0/message/message.h>
#include <b0/message/graph_link.h>

namespace b0
{

namespace message
{

/*!
 * \brief A complete graph of the network
 *
 * \sa GraphLink, \ref protocol, \ref graph
 */
class Graph : public Message
{
public:
    //! List of node names
    std::vector<std::string> nodes;

    //! List of topic links
    std::vector<GraphLink> node_topic;

    //! List of service links
    std::vector<GraphLink> node_service;

public:
    std::string type() const override;

private:
    void serialize(serialization::MessageFields &fields) const override;
};

} // namespace message

} // namespace b0

#endif // B0__MESSAGE__GRAPH_H__INCLUDED

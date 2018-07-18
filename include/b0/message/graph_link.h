#ifndef B0__MESSAGE__GRAPH_LINK_H__INCLUDED
#define B0__MESSAGE__GRAPH_LINK_H__INCLUDED

#include <boost/serialization/string.hpp>

#include <b0/message/message.h>

namespace b0
{

namespace message
{

/*!
 * \brief A link in the network
 *
 * \sa Graph, \ref protocol, \ref graph
 */
class GraphLink : public Message
{
public:
    //! The name of the node
    std::string node_name;

    //! The name of the topic or service
    std::string other_name;

    //! Direction of link: node->other if false, reversed otherwise
    bool reversed;

public:
    std::string type() const override;

private:
    void serialize(serialization::MessageFields &fields) const override;
};

} // namespace message

} // namespace b0

#endif // B0__MESSAGE__GRAPH_LINK_H__INCLUDED

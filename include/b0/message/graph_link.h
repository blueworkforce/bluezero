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
    std::string type() const override {return "GraphLink";}
};

} // namespace message

} // namespace b0

//! \cond HIDDEN_SYMBOLS

namespace spotify
{

namespace json
{

using b0::message::GraphLink;

template <>
struct default_codec_t<GraphLink>
{
    static codec::object_t<GraphLink> codec()
    {
        auto codec = codec::object<GraphLink>();
        codec.required("node_name", &GraphLink::node_name);
        codec.required("other_name", &GraphLink::other_name);
        codec.required("reversed", &GraphLink::reversed);
        return codec;
    }
};

} // namespace json

} // namespace spotify

//! \endcond

#endif // B0__MESSAGE__GRAPH_LINK_H__INCLUDED

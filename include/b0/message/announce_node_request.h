#ifndef B0__MESSAGE__ANNOUNCE_NODE_REQUEST_H__INCLUDED
#define B0__MESSAGE__ANNOUNCE_NODE_REQUEST_H__INCLUDED

#include <boost/serialization/string.hpp>

#include <b0/message/message.h>

namespace b0
{

namespace message
{

/*!
 * \brief Sent by node to resolver, to announce its presence and try to self-assign a name
 * 
 * \mscfile node-startup.msc
 *
 * The node name is only tentative. The AnnounceNodeResponse will tell the final node name.
 * 
 * \sa AnnounceNodeResponse, \ref protocol
 */
class AnnounceNodeRequest : public Message
{
public:
    //! The name of the node
    std::string node_name;

public:
    std::string type() const override {return "AnnounceNodeRequest";}
};

} // namespace message

} // namespace b0

//! \cond HIDDEN_SYMBOLS

namespace spotify
{

namespace json
{

using b0::message::AnnounceNodeRequest;

template <>
struct default_codec_t<AnnounceNodeRequest>
{
    static codec::object_t<AnnounceNodeRequest> codec()
    {
        auto codec = codec::object<AnnounceNodeRequest>();
        codec.required("node_name", &AnnounceNodeRequest::node_name);
        return codec;
    }
};

} // namespace json

} // namespace spotify

//! \endcond

#endif // B0__MESSAGE__ANNOUNCE_NODE_REQUEST_H__INCLUDED

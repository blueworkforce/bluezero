#ifndef B0__MESSAGE__ANNOUNCE_NODE_RESPONSE_H__INCLUDED
#define B0__MESSAGE__ANNOUNCE_NODE_RESPONSE_H__INCLUDED

#include <boost/serialization/string.hpp>

#include <b0/message/message.h>

namespace b0
{

namespace message
{

/*!
 * \brief Sent by resolver in reply to AnnounceNodeRequest message
 *
 * \mscfile node-startup.msc
 *
 * It assigns node's final name and gives the socket addresses for XPUB/XSUB proxies.
 *
 * \sa AnnounceNodeRequest, \ref protocol
 */
class AnnounceNodeResponse : public Message
{
public:
    //! True if successful, false if error. Should abort if false.
    bool ok;

    //! The final name of the node
    std::string node_name;

    //! Address of the XSUB zmq socket
    std::string xsub_sock_addr;

    //! Address of the XPUB zmq socket
    std::string xpub_sock_addr;

public:
    std::string type() const override {return "AnnounceNodeResponse";}
};

} // namespace message

} // namespace b0

//! \cond HIDDEN_SYMBOLS

namespace spotify
{

namespace json
{

using b0::message::AnnounceNodeResponse;

template <>
struct default_codec_t<AnnounceNodeResponse>
{
    static codec::object_t<AnnounceNodeResponse> codec()
    {
        auto codec = codec::object<AnnounceNodeResponse>();
        codec.required("ok", &AnnounceNodeResponse::ok);
        codec.required("node_name", &AnnounceNodeResponse::node_name);
        codec.required("xsub_sock_addr", &AnnounceNodeResponse::xsub_sock_addr);
        codec.optional("xpub_sock_addr", &AnnounceNodeResponse::xpub_sock_addr);
        return codec;
    }
};

} // namespace json

} // namespace spotify

//! \endcond

#endif // B0__MESSAGE__ANNOUNCE_NODE_RESPONSE_H__INCLUDED

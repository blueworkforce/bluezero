#ifndef B0__MESSAGE__SHUTDOWN_NODE_RESPONSE_H__INCLUDED
#define B0__MESSAGE__SHUTDOWN_NODE_RESPONSE_H__INCLUDED

#include <b0/message/message.h>

namespace b0
{

namespace message
{

/*!
 * \brief Sent by resolver to node in reply to ShutdownNodeRequest
 *
 * \mscfile node-shutdown.msc
 *
 * \sa ShutdownNodeRequest, \ref protocol
 */
class ShutdownNodeResponse : public Message
{
public:
    //! True fi successful, false if error
    bool ok;

public:
    std::string type() const override {return "ShutdownNodeResponse";}
};

} // namespace message

} // namespace b0

//! \cond HIDDEN_SYMBOLS

namespace spotify
{

namespace json
{

using b0::message::ShutdownNodeResponse;

template <>
struct default_codec_t<ShutdownNodeResponse>
{
    static codec::object_t<ShutdownNodeResponse> codec()
    {
        auto codec = codec::object<ShutdownNodeResponse>();
        codec.required("ok", &ShutdownNodeResponse::ok);
        return codec;
    }
};

} // namespace json

} // namespace spotify

//! \endcond

#endif // B0__MESSAGE__SHUTDOWN_NODE_RESPONSE_H__INCLUDED

#ifndef B0__MESSAGE__RESOLVE_SERVICE_REQUEST_H__INCLUDED
#define B0__MESSAGE__RESOLVE_SERVICE_REQUEST_H__INCLUDED

#include <boost/serialization/string.hpp>

#include <b0/message/message.h>

namespace b0
{

namespace message
{

/*!
 * \brief Sent by a ServiceClient to resolve a service name to a ZeroMQ address
 *
 * \mscfile service-resolve.msc
 *
 * \sa ResolveServiceResponse, \ref protocol
 */
class ResolveServiceRequest : public Message
{
public:
    //! The name of the service to be resolved
    std::string service_name;

public:
    std::string type() const override {return "ResolveServiceRequest";}
};

} // namespace message

} // namespace b0

//! \cond HIDDEN_SYMBOLS

namespace spotify
{

namespace json
{

using b0::message::ResolveServiceRequest;

template <>
struct default_codec_t<ResolveServiceRequest>
{
    static codec::object_t<ResolveServiceRequest> codec()
    {
        auto codec = codec::object<ResolveServiceRequest>();
        codec.required("service_name", &ResolveServiceRequest::service_name);
        return codec;
    }
};

} // namespace json

} // namespace spotify

//! \endcond

#endif // B0__MESSAGE__RESOLVE_SERVICE_REQUEST_H__INCLUDED

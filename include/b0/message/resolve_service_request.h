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
    std::string type() const override;

private:
    void serialize(serialization::MessageFields &fields) const override;
};

} // namespace message

} // namespace b0

#endif // B0__MESSAGE__RESOLVE_SERVICE_REQUEST_H__INCLUDED

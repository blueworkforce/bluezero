#ifndef B0__MESSAGE__RESOLVE_SERVICE_RESPONSE_H__INCLUDED
#define B0__MESSAGE__RESOLVE_SERVICE_RESPONSE_H__INCLUDED

#include <boost/serialization/string.hpp>

#include <b0/message/message.h>

namespace b0
{

namespace message
{

/*!
 * \brief Response to ResolveServiceRequest message
 *
 * \mscfile service-resolve.msc
 *
 * \sa ResolveServiceRequest, \ref protocol
 */
class ResolveServiceResponse : public Message
{
public:
    //! True if successful, false if error (i.e. does not exist)
    bool ok;

    //! The name of the zmq socket
    std::string sock_addr;

public:
    std::string type() const override;

private:
    void serialize(serialization::MessageFields &fields) const override;
};

} // namespace message

} // namespace b0

#endif // B0__MESSAGE__RESOLVE_SERVICE_RESPONSE_H__INCLUDED

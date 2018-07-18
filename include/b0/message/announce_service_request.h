#ifndef B0__MESSAGE__ANNOUNCE_SERVICE_REQUEST_H__INCLUDED
#define B0__MESSAGE__ANNOUNCE_SERVICE_REQUEST_H__INCLUDED

#include <boost/serialization/string.hpp>

#include <b0/message/message.h>

namespace b0
{

namespace message
{

/*!
 * \brief Sent by ServiceServer to announce a service by some name
 *
 * \mscfile node-startup-service.msc
 *
 * The service name must be unique.
 *
 * \sa AnnounceServiceResponse, \ref protocol
 */
class AnnounceServiceRequest : public Message
{
public:
    //! The name of the node
    std::string node_name;

    //! The name of the service
    std::string service_name;

    //! The address of the zmq socket
    std::string sock_addr;

public:
    std::string type() const override;

private:
    void serialize(serialization::MessageFields &fields) const override;
};

} // namespace message

} // namespace b0

#endif // B0__MESSAGE__ANNOUNCE_SERVICE_REQUEST_H__INCLUDED

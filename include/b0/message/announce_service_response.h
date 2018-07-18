#ifndef B0__MESSAGE__ANNOUNCE_SERVICE_RESPONSE_H__INCLUDED
#define B0__MESSAGE__ANNOUNCE_SERVICE_RESPONSE_H__INCLUDED

#include <b0/message/message.h>

namespace b0
{

namespace message
{

/*!
 * \brief Response to AnnounceServiceRequest message
 *
 * \mscfile node-startup-service.msc
 *
 * \sa AnnounceServiceRequest, \ref protocol
 */
class AnnounceServiceResponse : public Message
{
public:
    //! True if successful, false if error
    bool ok;

public:
    std::string type() const override;

private:
    void serialize(serialization::MessageFields &fields) const override;
};

} // namespace message

} // namespace b0

#endif // B0__MESSAGE__ANNOUNCE_SERVICE_RESPONSE_H__INCLUDED

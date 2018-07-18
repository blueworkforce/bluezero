#ifndef B0__MESSAGE__HEARTBEAT_RESPONSE_H__INCLUDED
#define B0__MESSAGE__HEARTBEAT_RESPONSE_H__INCLUDED

#include <b0/message/message.h>

namespace b0
{

namespace message
{

/*!
 * \brief Response to HeartBeatRequest message
 *
 * \mscfile node-lifetime.msc
 *
 * \sa HeartBeatRequest, \ref protocol
 */
class HeartbeatResponse : public Message
{
public:
    //! True if successful, false if error
    bool ok;

    //! Time stamp of the message
    int64_t time_usec;

public:
    std::string type() const override;

private:
    void serialize(serialization::MessageFields &fields) const override;
};

} // namespace message

} // namespace b0

#endif // B0__MESSAGE__HEARTBEAT_RESPONSE_H__INCLUDED

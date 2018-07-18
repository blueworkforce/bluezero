#ifndef B0__MESSAGE__HEARTBEAT_REQUEST_H__INCLUDED
#define B0__MESSAGE__HEARTBEAT_REQUEST_H__INCLUDED

#include <boost/serialization/string.hpp>

#include <b0/message/message.h>

namespace b0
{

namespace message
{

/*!
 * \brief Heartbeat periodically sent by node to resolver
 *
 * \mscfile node-lifetime.msc
 *
 * \sa HeartBeatResponse, \ref protocol
 */
class HeartbeatRequest : public Message
{
public:
    //! The name of the node
    std::string node_name;

public:
    std::string type() const override;

private:
    void serialize(serialization::MessageFields &fields) const override;
};

} // namespace message

} // namespace b0

#endif // B0__MESSAGE__HEARTBEAT_REQUEST_H__INCLUDED

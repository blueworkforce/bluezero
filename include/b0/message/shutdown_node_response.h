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
    std::string type() const override;

private:
    void serialize(serialization::MessageFields &fields) const override;
};

} // namespace message

} // namespace b0

#endif // B0__MESSAGE__SHUTDOWN_NODE_RESPONSE_H__INCLUDED

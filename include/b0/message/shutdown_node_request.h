#ifndef B0__MESSAGE__SHUTDOWN_NODE_REQUEST_H__INCLUDED
#define B0__MESSAGE__SHUTDOWN_NODE_REQUEST_H__INCLUDED

#include <boost/serialization/string.hpp>

#include <b0/message/message.h>

namespace b0
{

namespace message
{

/*!
 * \brief Sent by node to resolver when shutting down
 *
 * (not really a request, just a notification)
 *
 * \mscfile node-shutdown.msc
 *
 * \sa ShutdownNodeResponse, \ref protocol
 */
class ShutdownNodeRequest : public Message
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

#endif // B0__MESSAGE__SHUTDOWN_NODE_REQUEST_H__INCLUDED

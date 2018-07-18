#ifndef B0__MESSAGE__NODE_SERVICE_REQUEST_H__INCLUDED
#define B0__MESSAGE__NODE_SERVICE_REQUEST_H__INCLUDED

#include <boost/serialization/string.hpp>

#include <b0/message/message.h>

namespace b0
{

namespace message
{

/*!
 * \brief Sent by node to tell a service it is offering
 *
 * \mscfile graph-service.msc
 *
 * \sa NodeServiceResponse, \ref protocol, \ref graph
 */
class NodeServiceRequest : public Message
{
public:
    //! The name of the node
    std::string node_name;

    //! The name of the service
    std::string service_name;

    //! If true, node is a client, otherwise a server
    bool reverse;

    //! If true, the relationship is starting, otherwise it is ending
    bool active;

public:
    std::string type() const override;

private:
    void serialize(serialization::MessageFields &fields) const override;
};

} // namespace message

} // namespace b0

#endif // B0__MESSAGE__NODE_SERVICE_REQUEST_H__INCLUDED

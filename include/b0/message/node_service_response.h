#ifndef B0__MESSAGE__NODE_SERVICE_RESPONSE_H__INCLUDED
#define B0__MESSAGE__NODE_SERVICE_RESPONSE_H__INCLUDED

#include <b0/message/message.h>

namespace b0
{

namespace message
{

/*!
 * \brief Response to NodeServiceRequest message
 *
 * \mscfile graph-service.msc
 *
 * \sa NodeServiceRequest, \ref protocol, \ref graph
 */
class NodeServiceResponse : public Message
{
public:

public:
    std::string type() const override;

private:
    void serialize(serialization::MessageFields &fields) const override;
};

} // namespace message

} // namespace b0

#endif // B0__MESSAGE__NODE_SERVICE_RESPONSE_H__INCLUDED

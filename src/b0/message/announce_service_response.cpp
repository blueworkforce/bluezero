#include <b0/message/announce_service_response.h>

namespace b0
{

namespace message
{

std::string AnnounceServiceResponse::type() const
{
    return "AnnounceServiceResponse";
}

void AnnounceServiceResponse::serialize(serialization::MessageFields &fields) const
{
    fields.map("ok", &ok);
}

} // namespace message

} // namespace b0


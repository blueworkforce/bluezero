#include <b0/exception/message_unpack_error.h>

namespace b0
{

namespace exception
{

MessageUnpackError::MessageUnpackError(std::string message)
    : Exception(message)
{
}

} // namespace exception

} // namespace b0


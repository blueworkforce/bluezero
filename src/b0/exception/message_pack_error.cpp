#include <b0/exception/message_pack_error.h>

namespace b0
{

namespace exception
{

MessagePackError::MessagePackError(std::string message)
    : Exception(message)
{
}

EnvelopeEncodeError::EnvelopeEncodeError()
    : MessagePackError("Failed to encode message envelope (Protobuf serialize error)")
{
}

ProtobufSerializeError::ProtobufSerializeError()
    : MessagePackError("Failed to encode payload (Protobuf serialize error)")
{
}

SocketWriteError::SocketWriteError()
    : MessagePackError("Socket write error (send() failed)")
{
}

} // namespace exception

} // namespace b0


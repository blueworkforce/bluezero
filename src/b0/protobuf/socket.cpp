#include <b0/protobuf/socket.h>
#include <b0/exceptions.h>
#include <b0/config.h>

namespace b0
{

namespace protobuf
{

void SocketProtobuf::read(Socket *socket, google::protobuf::Message &msg)
{
    std::string payload, type;
    socket->readRaw(payload, type);
    if(!msg.ParseFromString(payload))
        throw exception::ProtobufParseError();
    std::string expected_type = msg.GetTypeName();
    if(type != expected_type)
        throw exception::MessageTypeMismatch(type, expected_type);
}

void SocketProtobuf::write(Socket *socket, const google::protobuf::Message &msg)
{
    std::string payload;
    if(!msg.SerializeToString(&payload))
        throw exception::ProtobufSerializeError();
    std::string type = msg.GetTypeName();
    socket->writeRaw(payload, type);
}

} // namespace protobuf

} // namespace b0


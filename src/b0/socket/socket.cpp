#include <b0/socket/socket.h>
#include <b0/config.h>
#include <b0/node.h>
#include <b0/exceptions.h>
#include <b0/compress/compress.h>

#include "resolver.pb.h"
#include "logger.pb.h"
#include "envelope.pb.h"

#include <boost/lexical_cast.hpp>

namespace b0
{

namespace socket
{

Socket::Socket(Node *node, zmq::socket_type type, std::string name, bool managed)
    : node_(*node),
      type_(type),
      name_(name),
      has_header_(false),
      managed_(managed),
      socket_(node_.getZMQContext(), type_)
{
    if(managed_)
        node_.addSocket(this);
}

Socket::~Socket()
{
    if(managed_)
        node_.removeSocket(this);
}

void Socket::spinOnce()
{
}

void Socket::setHasHeader(bool has_header)
{
    has_header_ = has_header;
}

void Socket::log(LogLevel level, std::string message) const
{
    if(boost::lexical_cast<std::string>(boost::this_thread::get_id()) == node_.threadID())
        node_.log(level, message);
    else
        std::cout << "[Skipped logger because in another thread]: " << message << std::endl;
}

void Socket::setRemoteAddress(std::string addr)
{
    remote_addr_ = addr;
}

std::string Socket::getName() const
{
    return name_;
}

void Socket::readRaw(std::string &msg)
{
    std::string type;
    readRaw(msg, type);
}

void Socket::readRaw(std::string &msg, std::string &type)
{
    std::string hdr, payload;

    // if necessary, read header (usually for topics, i.e. PUB/SUB)
    if(has_header_)
    {
        zmq::message_t msg_hdr;
        if(!socket_.recv(&msg_hdr))
            throw exception::SocketReadError();
        hdr = std::string(static_cast<char*>(msg_hdr.data()), msg_hdr.size());
        if(!msg_hdr.more())
            throw exception::MessageMissingHeaderError();
    }
    
    // read payload
    zmq::message_t msg_payload;
    if(!socket_.recv(&msg_payload))
        throw exception::SocketReadError();
    payload = std::string(static_cast<char*>(msg_payload.data()), msg_payload.size());

    // read additional parts, and throw an error, because there shouldn't be any more parts
    if(msg_payload.more())
    {
        bool more = true;
        while(more)
        {
            zmq::message_t msg_tmp;
            if(!socket_.recv(&msg_tmp))
                throw exception::SocketReadError();
            more = msg_tmp.more();
        }
        throw exception::MessageTooManyPartsError();
    }

    // if necessary, check header
    if(has_header_ && hdr != name_)
        throw exception::HeaderMismatch(hdr, name_);

    b0::core_msgs::MessageEnvelope env;
    if(!env.ParseFromString(payload))
        throw exception::EnvelopeDecodeError();
    msg = b0::compress::decompress(env.compression_algorithm(), env.payload(), env.uncompressed_size());
    type = env.type();
}

void Socket::read(google::protobuf::Message &msg)
{
    std::string payload, type;
    readRaw(payload, type);
    if(!msg.ParseFromString(payload))
        throw exception::ProtobufParseError();
    std::string expected_type = msg.GetTypeName();
    if(type != expected_type)
        throw exception::MessageTypeMismatch(type, expected_type);
}

bool Socket::poll(long timeout)
{
#ifdef __GNUC__
    zmq::pollitem_t items[] = {{static_cast<void*>(socket_), 0, ZMQ_POLLIN, 0}};
#else
    zmq::pollitem_t items[] = {{socket_, 0, ZMQ_POLLIN, 0}};
#endif
    zmq::poll(&items[0], sizeof(items) / sizeof(items[0]), timeout);
    return items[0].revents & ZMQ_POLLIN;
}

void Socket::writeRaw(const std::string &msg, const std::string &type)
{
    // if necessary, write header (usually for topics, i.e. PUB/SUB)
    if(has_header_)
    {
        zmq::message_t msg_hdr(name_.size());
        std::memcpy(msg_hdr.data(), name_.data(), name_.size());
        if(!socket_.send(msg_hdr, ZMQ_SNDMORE))
            throw exception::SocketWriteError();
    }

    // create payload envelope
    b0::core_msgs::MessageEnvelope env;
    env.set_uncompressed_size(msg.size());
    env.set_compression_algorithm(compression_algorithm_);
    env.set_payload(b0::compress::compress(compression_algorithm_, msg, compression_level_));
    env.set_type(type);
    std::string payload;
    if(!env.SerializeToString(&payload))
        throw exception::EnvelopeEncodeError();

    // write payload
    zmq::message_t msg_payload(payload.size());
    std::memcpy(msg_payload.data(), payload.data(), payload.size());
    if(!socket_.send(msg_payload))
        throw exception::SocketWriteError();
}

void Socket::write(const google::protobuf::Message &msg)
{
    std::string payload;
    if(!msg.SerializeToString(&payload))
        throw exception::ProtobufSerializeError();
    std::string type = msg.GetTypeName();
    writeRaw(payload, type);
}

void Socket::setCompression(std::string algorithm, int level)
{
    compression_algorithm_ = algorithm;
    compression_level_ = level;
}

} // namespace socket

} // namespace b0


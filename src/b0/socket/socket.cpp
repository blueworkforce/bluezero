#include <b0/socket/socket.h>
#include <b0/config.h>
#include <b0/node.h>
#include <b0/exceptions.h>
#include <b0/compress/compress.h>

#include "envelope.pb.h"

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

namespace b0
{

namespace socket
{

std::string wrapEnvelope(std::string payload, std::string compression_algorithm, int compression_level)
{
    b0::core_msgs::MessageEnvelope env;
    env.set_uncompressed_size(payload.size());
    env.set_compression_algorithm(compression_algorithm);
    env.set_payload(b0::compress::compress(compression_algorithm, payload, compression_level));
    std::string raw;
    env.SerializeToString(&raw);
    return raw;
}

std::string unwrapEnvelope(std::string rawData)
{
    b0::core_msgs::MessageEnvelope env;
    if(!env.ParseFromString(rawData))
        return "";
    return b0::compress::decompress(env.compression_algorithm(), env.payload(), env.uncompressed_size());
}

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

bool Socket::read(std::string &msg)
{
    std::string hdr, payload;

    bool ok = true;

    if(has_header_)
    {
        zmq::message_t msg_hdr;
        ok = socket_.recv(&msg_hdr) && ok;
        hdr = std::string(static_cast<char*>(msg_hdr.data()), msg_hdr.size());
        if(!msg_hdr.more())
            throw exception::MessageUnpackError("expected a multipart message");
    }
    
    zmq::message_t msg_payload;
    ok = socket_.recv(&msg_payload) && ok;
    payload = std::string(static_cast<char*>(msg_payload.data()), msg_payload.size());

    if(msg_payload.more())
    {
        bool more = true;
        while(more)
        {
            zmq::message_t msg_tmp;
            socket_.recv(&msg_tmp);
            more = msg_tmp.more();
        }
        throw exception::MessageUnpackError("too many message parts");
    }

    if(has_header_ && hdr != name_)
    {
        boost::format fmt("message header does not match: expected '%s', got '%s'");
        throw exception::MessageUnpackError((fmt % name_ % hdr).str());
        //return false;
    }

    msg = unwrapEnvelope(payload);

    return ok;
}

bool Socket::read(google::protobuf::Message &msg)
{
    std::string payload;
    return read(payload) && msg.ParseFromString(payload);
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

bool Socket::write(const std::string &msg)
{
    std::string payload = wrapEnvelope(msg, compression_algorithm_, compression_level_);

    bool ok = true;

    if(has_header_)
    {
        zmq::message_t msg_hdr(name_.size());
        std::memcpy(msg_hdr.data(), name_.data(), name_.size());
        ok = socket_.send(msg_hdr, ZMQ_SNDMORE) && ok;
    }

    zmq::message_t msg_payload(payload.size());
    std::memcpy(msg_payload.data(), payload.data(), payload.size());
    ok = socket_.send(msg_payload) && ok;

    return ok;
}

bool Socket::write(const google::protobuf::Message &msg)
{
    std::string payload;
    return msg.SerializeToString(&payload) && write(payload);
}

void Socket::setCompression(std::string algorithm, int level)
{
    compression_algorithm_ = algorithm;
    compression_level_ = level;
}

} // namespace socket

} // namespace b0


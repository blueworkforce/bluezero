#include <b0/socket.h>
#include <b0/config.h>
#include <b0/node.h>
#include <b0/exceptions.h>
#include <b0/message/message_envelope.h>
#include <b0/compress/compress.h>
#include <b0/utils/env.h>

#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <zmq.hpp>

namespace b0
{

struct SocketPrivate
{
    SocketPrivate(Node *node, zmq::context_t &context, int type)
        : type_(type),
          socket_(context, type)
    {
    }

    int type_;
    zmq::socket_t socket_;
};

Socket::Socket(Node *node, int type, std::string name, bool managed)
    : private_(new SocketPrivate(node, *reinterpret_cast<zmq::context_t*>(node->getContext()), type)),
      node_(*node),
      name_(name),
      has_header_(false),
      managed_(managed)
{
    setLingerPeriod(5000);

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

Node & Socket::getNode() const
{
    return node_;
}

bool Socket::matchesPattern(const std::string &pattern) const
{
    if(pattern == "*") return true;
    size_t pos = pattern.find('.');
    if(pos != std::string::npos)
    {
        std::string np = pattern.substr(0, pos);
        std::string sp = pattern.substr(pos + 1);
        return (np == "*" || np == getNode().getName())
            && (sp == "*" || sp == getName());
    }
    return false;
}

void Socket::readRaw(std::string &msg)
{
    std::string type;
    readRaw(msg, type);
}

static void dumpPayload(const Socket &socket, const std::string &op, const std::string &payload)
{
    // to enable debug for a socket, set B0_DEBUG_SOCKET to nodeName.sockName
    // wildcards can be used (e.g.: *.sockName, nodeName.*, *.*, *)
    // multiple patterns can be specified, using ':' as a separator
    std::string debug_socket = b0::env::get("B0_DEBUG_SOCKET");
    std::vector<std::string> debug_socket_v;
    boost::split(debug_socket_v, debug_socket, boost::is_any_of(":;"));
    bool debug_enabled = false;
    for(auto &x : debug_socket_v) if(socket.matchesPattern(x)) {debug_enabled = true; break;}
    if(!debug_enabled) return;

    std::stringstream dbg;
    dbg << "B0_DEBUG_SOCKET[sock=" << socket.getNode().getName() << "." << socket.getName() << ", op=" << op << ", len=" << payload.size() << "]: ";
    for(size_t i = 0; i < payload.size(); i++)
    {
        unsigned char c = payload[i];
        if(c == '\n')
            dbg << "\\n";
        else if(c == '\r')
            dbg << "\\r";
        else if(c == '\t')
            dbg << "\\t";
        else if(c < 32 || c > 126)
            dbg << boost::format("\\x%02x") % int(c);
        else
            dbg << char(c);
    }
    std::cout << dbg.str() << std::endl;
}

void Socket::readRaw(std::string &msg, std::string &type)
{
    zmq::socket_t &socket_ = private_->socket_;

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

    dumpPayload(*this, "recv", payload);
    b0::message::MessageEnvelope env;
    env.parseFromString(payload);
    msg = b0::compress::decompress(env.compression_algorithm, env.payload, env.content_length);
    type = env.content_type;
}

void Socket::readMsg(b0::message::Message &msg)
{
    std::string str, type;
    readRaw(str, type);
    if(type != msg.type())
        throw exception::EnvelopeDecodeError();
    msg.parseFromString(str);
}

bool Socket::poll(long timeout)
{
    zmq::socket_t &socket_ = private_->socket_;
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
    zmq::socket_t &socket_ = private_->socket_;

    // if necessary, write header (usually for topics, i.e. PUB/SUB)
    if(has_header_)
    {
        zmq::message_t msg_hdr(name_.size());
        std::memcpy(msg_hdr.data(), name_.data(), name_.size());
        if(!socket_.send(msg_hdr, ZMQ_SNDMORE))
            throw exception::SocketWriteError();
    }

    // create payload envelope
    b0::message::MessageEnvelope env;
    env.content_length = msg.size();
    env.compression_algorithm = compression_algorithm_;
    env.payload = b0::compress::compress(compression_algorithm_, msg, compression_level_);
    env.content_type = type;
    std::string payload;
    env.serializeToString(payload);
    dumpPayload(*this, "send", payload);

    // write payload
    zmq::message_t msg_payload(payload.size());
    std::memcpy(msg_payload.data(), payload.data(), payload.size());
    if(!socket_.send(msg_payload))
        throw exception::SocketWriteError();
}

void Socket::writeMsg(const b0::message::Message &msg)
{
    std::string str, type;
    type = msg.type();
    msg.serializeToString(str);
    writeRaw(str, type);
}

void Socket::setCompression(std::string algorithm, int level)
{
    compression_algorithm_ = algorithm;
    compression_level_ = level;
}

int Socket::getReadTimeout() const
{
    return getIntOption(ZMQ_RCVTIMEO);
}

void Socket::setReadTimeout(int timeout)
{
    setIntOption(ZMQ_RCVTIMEO, timeout);
}

int Socket::getWriteTimeout() const
{
    return getIntOption(ZMQ_SNDTIMEO);
}

void Socket::setWriteTimeout(int timeout)
{
    setIntOption(ZMQ_SNDTIMEO, timeout);
}

int Socket::getLingerPeriod() const
{
    return getIntOption(ZMQ_LINGER);
}

void Socket::setLingerPeriod(int period)
{
    setIntOption(ZMQ_LINGER, period);
}

int Socket::getBacklog() const
{
    return getIntOption(ZMQ_BACKLOG);
}

void Socket::setBacklog(int backlog)
{
    setIntOption(ZMQ_BACKLOG, backlog);
}

bool Socket::getImmediate() const
{
    return getIntOption(ZMQ_IMMEDIATE) != 0;
}

void Socket::setImmediate(bool immediate)
{
    setIntOption(ZMQ_IMMEDIATE, immediate ? 1 : 0);
}

bool Socket::getConflate() const
{
    return getIntOption(ZMQ_CONFLATE) != 0;
}

void Socket::setConflate(bool conflate)
{
    setIntOption(ZMQ_CONFLATE, conflate ? 1 : 0);
}

int Socket::getReadHWM() const
{
    return getIntOption(ZMQ_RCVHWM);
}

void Socket::setReadHWM(int n)
{
    setIntOption(ZMQ_RCVHWM, n);
}

int Socket::getWriteHWM() const
{
    return getIntOption(ZMQ_SNDHWM);
}

void Socket::setWriteHWM(int n)
{
    setIntOption(ZMQ_SNDHWM, n);
}

void Socket::connect(std::string const &addr)
{
    zmq::socket_t &socket_ = private_->socket_;
    socket_.connect(addr);
}

void Socket::disconnect(std::string const &addr)
{
    zmq::socket_t &socket_ = private_->socket_;
    socket_.disconnect(addr);
}

void Socket::bind(std::string const &addr)
{
    zmq::socket_t &socket_ = private_->socket_;
    socket_.bind(addr);
}

void Socket::unbind(std::string const &addr)
{
    zmq::socket_t &socket_ = private_->socket_;
    socket_.unbind(addr);
}

void Socket::setsockopt(int option, const void *optval, size_t optvallen)
{
    zmq::socket_t &socket_ = private_->socket_;
    socket_.setsockopt(option, optval, optvallen);
}

void Socket::getsockopt(int option, void *optval, size_t *optvallen) const
{
    zmq::socket_t &socket_ = private_->socket_;
    socket_.getsockopt(option, optval, optvallen);
}

void Socket::setIntOption(int option, int value)
{
    zmq::socket_t &socket_ = private_->socket_;
    socket_.setsockopt<int>(option, value);
}

int Socket::getIntOption(int option) const
{
    zmq::socket_t &socket_ = private_->socket_;
    return socket_.getsockopt<int>(option);
}

} // namespace b0


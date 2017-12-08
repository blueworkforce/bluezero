#ifndef B0__SOCKET__SOCKET_H__INCLUDED
#define B0__SOCKET__SOCKET_H__INCLUDED

#include <string>

#include <b0/graph/graph.h>
#include <b0/logger/interface.h>

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <zmq.hpp>

#include <google/protobuf/message.h>

namespace b0
{

class Node;

namespace socket
{

/*!
 * \brief The Socket class
 *
 * This class wraps a ZeroMQ socket. It provides wrappers for reading and writing
 * raw payloads, as well as google::protobuf messages.
 *
 * \sa b0::Publisher, b0::Subscriber, b0::ServiceClient, b0::ServiceServer
 */
class Socket : public logger::LogInterface
{
public:
    /*!
     * \brief Construct a Socket
     */
    Socket(Node *node, zmq::socket_type type, std::string name, bool managed = true);

    /*!
     * \brief Socket destructor
     */
    virtual ~Socket();

    /*!
     * \brief Set the has_header_ flag which specifies if this socket require a message part
     *        with the address (usually for publish/subscribe pattern).
     */
    void setHasHeader(bool has_header);

    /*!
     * \brief Log a message to the default logger of this node
     */
    void log(LogLevel level, std::string message) const override;

    /*!
     * \brief Perform initialization (resolve name, connect socket, set subscription)
     */
    virtual void init() = 0;

    /*!
     * \brief Perform cleanup (clear subscription, disconnect socket)
     */
    virtual void cleanup() = 0;

    /*!
     * \brief Process incoming messages and call callbacks
     */
    virtual void spinOnce() = 0;

    /*!
     * \brief Set the remote address the socket will connect to
     */
    void setRemoteAddress(std::string addr);

    /*!
     * \brief Return the name of the socket bus
     */
    std::string getName() const;

protected:
    //! The type of the underlying ZeroMQ socket
    zmq::socket_type type_;

    //! The Node owning this Socket
    Node &node_;

    //! This socket bus name
    std::string name_;

    //! \brief True if the payload has a header part (i.e. topic).
    //! If true, the envelope with this socket name will be prepended to the payload.
    bool has_header_;

    //! True if this socket is managed (init(), cleanup() are called by the owner Node)
    const bool managed_;

    //! The address of the ZeroMQ socket to connect to (will skip name resolution if given)
    std::string remote_addr_;

    //! The underlying ZeroMQ socket
    zmq::socket_t socket_;

public:
    /*!
     * \brief Read a raw payload from the underlying ZeroMQ socket
     */
    virtual bool read(std::string &msg);

    /*!
     * \brief Read a google::protobuf::Message from the underlying ZeroMQ socket
     */
    virtual bool read(google::protobuf::Message &msg);

    /*!
     * \brief Poll for messages. If timeout is 0 return immediately, otherwise wait
     *        for the specified amount of milliseconds.
     */
    virtual bool poll(long timeout = 0);

    /*!
     * \brief Write a raw payload
     */
    virtual bool write(const std::string &msg);

    /*!
     * \brief Write a google::protobuf::Message
     */
    virtual bool write(const google::protobuf::Message &msg);

public:
    /*!
     * \brief Set compression algorithm and level
     */
    void setCompression(std::string algorithm, int level = -1);

protected:
    //! If set, payloads will be encoded using the specified compression algorithm
    //! \sa WriteSocket::setCompression()
    std::string compression_algorithm_;

    //! If a compression algorithm is set, payloads will be encoded using the specified compression level
    //! \sa WriteSocket::setCompression()
    int compression_level_;
};

} // namespace socket

} // namespace b0

#endif // B0__SOCKET__SOCKET_H__INCLUDED

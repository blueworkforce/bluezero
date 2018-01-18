#ifndef B0__SOCKET_H__INCLUDED
#define B0__SOCKET_H__INCLUDED

#include <string>

#include <b0/user_data.h>
#include <b0/logger/interface.h>

#include <boost/function.hpp>
#include <boost/bind.hpp>

namespace b0
{

class Node;

//! \cond HIDDEN_SYMBOLS

struct SocketPrivate;

//! \endcond

/*!
 * \brief The Socket class
 *
 * This class wraps a ZeroMQ socket. It provides wrappers for reading and writing
 * raw payloads, as well as google::protobuf messages.
 *
 * \sa b0::Publisher, b0::Subscriber, b0::ServiceClient, b0::ServiceServer
 */
class Socket : public logger::LogInterface, public UserData
{
public:
    /*!
     * \brief Construct a Socket
     */
    Socket(Node *node, int type, std::string name, bool managed = true);

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
    virtual void spinOnce();

    /*!
     * \brief Set the remote address the socket will connect to
     */
    void setRemoteAddress(std::string addr);

    /*!
     * \brief Return the name of the socket bus
     */
    std::string getName() const;

private:
    std::unique_ptr<SocketPrivate> private_;

protected:
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

public:
    /*!
     * \brief Read a raw payload from the underlying ZeroMQ socket
     */
    virtual void readRaw(std::string &msg);

    /*!
     * \brief Read a raw payload with type from the underlying ZeroMQ socket
     */
    virtual void readRaw(std::string &msg, std::string &type);

    /*!
     * \brief Poll for messages. If timeout is 0 return immediately, otherwise wait
     *        for the specified amount of milliseconds.
     */
    virtual bool poll(long timeout = 0);

    /*!
     * \brief Write a raw payload
     */
    virtual void writeRaw(const std::string &msg, const std::string &type = "");

public:
    /*!
     * \brief Set compression algorithm and level
     *
     * The messages sent with this socket will be compressed using the specified algorithm.
     * This has no effect on received messages, which will be automatically decompressed
     * using the algorithm specified in the message envelope.
     */
    void setCompression(std::string algorithm, int level = -1);

private:
    //! If set, payloads will be encoded using the specified compression algorithm
    //! \sa WriteSocket::setCompression()
    std::string compression_algorithm_;

    //! If a compression algorithm is set, payloads will be encoded using the specified compression level
    //! \sa WriteSocket::setCompression()
    int compression_level_;

public:
    //! (low-level socket option) Get read timeout
    int getReadTimeout() const;

    //! (low-level socket option) Set read timeout
    void setReadTimeout(int timeout);

    //! (low-level socket option) Get write timeout
    int getWriteTimeout() const;

    //! (low-level socket option) Set write timeout
    void setWriteTimeout(int timeout);

    //! (low-level socket option) Get linger period
    int getLingerPeriod() const;

    //! (low-level socket option) Set linger period
    void setLingerPeriod(int period);

    //! (low-level socket option) Get backlog
    int getBacklog() const;

    //! (low-level socket option) Set backlog
    void setBacklog(int backlog);

    //! (low-level socket option) Get immediate flag
    bool getImmediate() const;

    //! (low-level socket option) Set immediate flag
    void setImmediate(bool immediate);

    //! (low-level socket option) Get conflate flag
    bool getConflate() const;

    //! (low-level socket option) Set conflate flag
    void setConflate(bool conflate);

    //! (low-level socket option) Get read high-water-mark
    int getReadHWM() const;

    //! (low-level socket option) Set read high-water-mark
    void setReadHWM(int n);

    //! (low-level socket option) Get write high-water-mark
    int getWriteHWM() const;

    //! (low-level socket option) Set write high-water-mark
    void setWriteHWM(int n);

protected:
    //! Wrapper to zmq::socket_t::connect
    void connect(std::string const &addr);

    //! Wrapper to zmq::socket_t::disconnect
    void disconnect(std::string const &addr);

    //! Wrapper to zmq::socket_t::bind
    void bind(std::string const &addr);

    //! Wrapper to zmq::socket_t::unbind
    void unbind(std::string const &addr);

    //! Wrapper to zmq::socket_t::setsockopt
    void setsockopt(int option, const void *optval, size_t optvallen);

    //! Wrapper to zmq::socket_t::getsockopt
    void getsockopt(int option, void *optval, size_t *optvallen) const;

    //! High level wrapper for setsockopt
    void setIntOption(int option, int value);

    //! High level wrapper for getsockopt
    int getIntOption(int option) const;
};

} // namespace b0

#endif // B0__SOCKET_H__INCLUDED

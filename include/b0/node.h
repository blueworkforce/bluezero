#ifndef B0__NODE_H__INCLUDED
#define B0__NODE_H__INCLUDED

#include <b0/user_data.h>
#include <b0/node_state.h>
#include <b0/socket.h>
#include <b0/logger/interface.h>
#include <b0/utils/time_sync.h>

#include <atomic>
#include <set>
#include <string>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

namespace b0
{

namespace logger
{

class Logger;

} // namespace logger

//! \cond HIDDEN_SYMBOLS

struct NodePrivate;
struct NodePrivate2;

//! \endcond

/*!
 * \brief The abstraction for a node in the network.
 *
 * You must create at most one node per thread.
 * You can have many nodes in one process by creating several threads.
 */
class Node : public logger::LogInterface, public UserData
{
public:
    using logger::LogInterface::log;

    /*!
     * \brief Create a node with a given name.
     * \param nodeName the name of the node
     * \sa getName()
     *
     * Create a node with a given name.
     *
     * A message will be send to resolver to announce this node presence on the network.
     * If a node with the same name already exists in the network, this node will get
     * a different name.
     *
     * The heartbeat thread will be started to let resolver track the online status of this node.
     */
    Node(std::string nodeName = "");

    /*!
     * \brief Destruct this node
     *
     * Any threads, such as the heartbeat thread, will be stopped, and the sockets
     * will be freeed.
     */
    virtual ~Node();

    /*!
     * \brief Initialize the node (connect to resolve, start heartbeat, announce node name)
     *
     * If you need to extend the init phase, when overriding it in your
     * subclass, remember to first call this Node::init() (unless you know what
     * you are doing).
     */
    virtual void init();

    /*!
     * \brief Shutdown the node (stop all running threads, send shutdown notification)
     *
     * If you need to perform additional cleanup, when overriding this method
     * in your subclass, remember to first call this Node::shutdown() (unless you know
     * what you are doing).
     */
    virtual void shutdown();

    /*!
     * \brief Return wether shutdown has requested (by Node::shutdown() method or by pressing CTRL-C)
     */
    bool shutdownRequested() const;

    /*!
     * \brief Read all available messages from the various ZeroMQ sockets, and
     * dispatch them to callbacks.
     *
     * This method will call b0::Subscriber::spinOnce() and b0::ServiceServer::spinOnce()
     * on the subscribers and service servers that belong to this node.
     *
     * Warning: every message sent on a topic which has no registered callback will be discarded.
     */
    virtual void spinOnce();

    /*!
     * \brief Run the spin loop (continuously call spinOnce(), at the specified rate, and call cleanup() at the end)
     *
     * \param spinRate the approximate frequency (in Hz) at which spinOnce() will be called
     */
    virtual void spin(double spinRate = 10.0);

    /*!
     * \brief Node cleanup: stop all threads, send a shutdown notification to resolver, and so on...
     */
    virtual void cleanup();

protected:
    /*!
     * \brief Start the heartbeat thread
     *
     * The heartbeat thread will periodically send a heartbeat message to inform the
     * resolver node that this node is alive.
     */
    virtual void startHeartbeatThread();

public:
    /*!
     * \brief Log a message to the default logger of this node
     */
    void log(LogLevel level, std::string message) const override;

    /*!
     * \brief Get the name assigned by resolver to this node
     */
    std::string getName() const;

    /*!
     * \brief Get the state of this node
     */
    NodeState getState() const;

    /*!
     * \brief Get the ZeroMQ Context
     */
    void * getContext();

    /*!
     * \brief Retrieve address of the proxy's XPUB socket
     */
    virtual std::string getXPUBSocketAddress() const;

    /*!
     * \brief Retrieve address of the proxy's XSUB socket
     */
    virtual std::string getXSUBSocketAddress() const;

private:
    /*!
     * Register a socket for this node. Do not call this directly. Called by Socket class.
     */
    void addSocket(Socket *socket);

    /*!
     * Register a socket for this node. Do not call this directly. Called by Socket class.
     */
    void removeSocket(Socket *socket);

public:
    /*!
     * \brief Return the public address (IP or hostname) to reach this node on the network
     */
    virtual std::string hostname();

    /*!
     * \brief Return the process id of this node
     */
    virtual int pid();

    /*!
     * \brief Return the thread identifier of this node.
     */
    virtual std::string threadID();

    /*!
     * \brief Find and return an available TCP port
     */
    virtual int freeTCPPort();

    /*!
     * \brief Notify topic publishing/subscription start or end
     */
    virtual void notifyTopic(std::string topic_name, bool reverse, bool active);

    /*!
     * \brief Notify service advertising/use start or end
     */
    virtual void notifyService(std::string service_name, bool reverse, bool active);

    /*!
     * \brief Announce service address
     */
    virtual void announceService(std::string service_name, std::string addr);

    /*!
     * \brief Resolve service address by name
     */
    virtual void resolveService(std::string service_name, std::string &addr);

    /*!
     * \brief Set the timeout for the announce phase. See b0::resolver::Client::setAnnounceTimeout()
     */
    virtual void setAnnounceTimeout(int timeout = -1);

protected:
    /*!
     * \brief Find and return an available tcp address, e.g. tcp://hostname:portnumber
     */
    virtual std::string freeTCPAddress();

    /*!
     * \brief Announce this node to resolver
     */
    virtual void announceNode();

    /*!
     * \brief Notify resolver of this node shutdown
     */
    virtual void notifyShutdown();

    /*!
     * \brief The heartbeat message loop (run in its own thread)
     */
    virtual void heartbeatLoop();

public:
    /*!
     * \brief Return this computer's clock time in microseconds
     */
    int64_t hardwareTimeUSec() const;

    /*!
     * \brief Return the adjusted time in microseconds. See \ref timesync for details.
     */
    int64_t timeUSec();

private:
    std::unique_ptr<NodePrivate> private_;
    std::unique_ptr<NodePrivate2> private2_;

protected:
    //! Target address of resolver client
    std::string resolv_addr_;

    //! The logger of this node
    logger::LogInterface *p_logger_;

private:
    //! Name of this node as it has been assigned by resolver
    std::string name_;

    //! State of this node
    NodeState state_;

    //! Id of the thread in which this node has been created
    boost::thread::id thread_id_;

    //! Heartbeat thread
    boost::thread heartbeat_thread_;

    //! List of sockets
    std::set<Socket*> sockets_;

    //! Address of the proxy's XSUB socket
    std::string xsub_sock_addr_;

    //! Address of the proxy's XPUB socket
    std::string xpub_sock_addr_;

    //! Flag set by the signal (SIGINT) handler
    static std::atomic<bool> quit_flag_;

    //! Flag set by Node::shutdown()
    bool shutdown_flag_;

    //! Flag to indicate wether the signal (SIGINT) handler has been set up
    static bool sigint_handler_setup_;

    //! Time synchronization object
    TimeSync time_sync_;

    //! Signal (SIGINT) handler
    static void signalHandler(int s);

    //! Routine to set up the signal (SIGINT) handler
    static void setupSIGINTHandler();

public:
    friend class Socket;
};

} // namespace b0

#endif // B0__NODE_H__INCLUDED

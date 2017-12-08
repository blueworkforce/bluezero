#ifndef B0__NODE_H__INCLUDED
#define B0__NODE_H__INCLUDED

#include <b0/socket/socket.h>
#include <b0/service_client.h>
#include <b0/logger/interface.h>

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

namespace resolver_msgs
{

class Request;
class Response;

} // namespace resolver_msgs

class AbstractPublisher;
class AbstractSubscriber;
class AbstractServiceClient;
class AbstractServiceServer;

/*!
 * \brief The abstraction for a node in the network.
 *
 * You must create at most one node per thread.
 * You can have many nodes in one process by creating several threads.
 */
class Node : public logger::LogInterface
{
public:
    using logger::LogInterface::log;

    /*!
     * \brief The state of a Node
     */
    enum State
    {
        //! \brief Just after creation, before initialization.
        Created,
        //! \brief After initialization.
        Ready,
        //! \brief Just after cleanup.
        Terminated
    };

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

    /*!
     * \brief Log a message to the default logger of this node
     */
    void log(LogLevel level, std::string message) const override;

public:
    /*!
     * \brief Get the name assigned by resolver to this node
     */
    std::string getName() const;

    /*!
     * \brief Get the state of this node
     */
    State getState() const;

    /*!
     * \brief Get the ZeroMQ Context
     */
    zmq::context_t& getZMQContext();

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
    void addSocket(socket::Socket *socket);

    /*!
     * Register a socket for this node. Do not call this directly. Called by Socket class.
     */
    void removeSocket(socket::Socket *socket);

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

protected:
    /*!
     * \brief Find and return an available tcp address, e.g. tcp://hostname:portnumber
     */
    virtual std::string freeTCPAddress();

    /*!
     * \brief Get the address of the resolver (e.g. may be provided via an env var)
     *
     * The resolver address must be provided via the BWF_RESOLVER environment variable.
     * If it is not provided, it will be automatically guessed.
     * The current way of guessing it is to simply return "tcp://localhost:22000", which
     * is only suitable for running everything on the same machine.
     */
    virtual std::string resolverAddress() const;

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
     * \page timesync Time Synchronization
     *
     * This page describes how time synchronization works.
     *
     * The objective of time synchronization is to coordinate otherwise independent clocks. Even when initially set accurately, real clocks will differ after some amount of time due to clock drift, caused by clocks counting time at slightly different rates.
     *
     * \image html timesync_plot1.png "Example of two drifting clocks" width=500pt
     *
     * There is one master clock node, which usualy coincides with the resolver node,
     * and every other node instance will try to synchronize its clock to the master clock,
     * while maintaining a guarrantee on some properties:
     *
     *  - time must not do arbitrarily big jumps
     *  - time must always increase monotonically, i.e. if we read time into variable T1, and after some time we read again time into variable T2, it must always be that T2 >= T1
     *  - locally, adjusted time must change at a constant speed, that is, the adjustment must happen at a constant rate
     *  - the adjustment must be a continuous function of time, such that even if the time is adjusted at a low rate (typically 1Hz) we get a consistent behavior for sub-second reads
     *
     *  Time synchronization never changes the computer's hardware clock.
     *  It rather computes an offset to add to the hardware clock.
     *
     *  The method Node::timeUSec() returns the value of the hardware clock corrected by the required offset, while the method Node::hardwareTimeUSec() will return the hardware clock actual value.
     *
     *  Each time a new time is received from master clock (tipically in the heartbeat message) the method Node::updateTime() is called, and a new offset is computed.
     *
     * \image html timesync_plot2.png "Example time series of the offset, which is computed as the difference between local time and remote time. Note that is not required that the offset is received at fixed intervals, and in fact in this example it is not the case." width=500pt
     *
     *  If we look at the offset as a function of time we see that is discontinuous.
     *  This is bad because just adding the offset to the hardware clock would cause
     *  arbitrarily big jumps and even jump backwards in time, thus violating the
     *  two properties stated before.
     *
     * \image html timesync_plot3.png "The adjusted time obtained by adding the offset to local time" width=500pt
     *
     *  To fix this, the offset function is smoothed so that it is continuous, and
     *  limited in its rate of change (max slope).
     *  It is important that the max slope is always greater than zero, so as to produce an actual change, and strictly less than 1, so as to not cause time to stop or go backwards.
     *
     * \image html timesync_plot4.png "The smoothed offset. In this example we used a max slope of 0.5, such that the time adjustment is at most half second per second." width=500pt
     *
     * \image html timesync_plot5.png "The resulting adjusted time" width=500pt
     *
     */

    /*!
     * \brief Return this computer's clock time in microseconds
     */
    int64_t hardwareTimeUSec() const;

    /*!
     * \brief Return the adjusted time in microseconds. See \ref timesync for details.
     */
    int64_t timeUSec();

protected:
    /*!
     * Compute a smoothed offset with a linear velocity profile
     * with a slope never greater (in absolute value) than max_slope
     */
    int64_t constantRateAdjustedOffset();

    /*!
     * Update the time offset with a time from remote server (in microseconds)
     */
    void updateTime(int64_t remoteTime);

    //! \cond HIDDEN_SYMBOLS

    /*!
     * State variables related to time synchronization
     */
    struct timesync {
        int64_t target_offset_;
        int64_t last_offset_time_;
        int64_t last_offset_value_;
        double max_slope_;
        boost::mutex mutex_;
    } timesync_;

    //! \endcond

public:
    //! An alias for the ServiceClient which talks to the resolver service
    using ResolverServiceClient = ServiceClient<b0::resolver_msgs::Request, b0::resolver_msgs::Response, false>;

    //! Return the ServiceClient to talk to resolver
    ResolverServiceClient & resolverClient() {return resolv_cli_;}

protected:
    //! ZeroMQ Context used by all sockets of this node
    zmq::context_t context_;

    //! Service client used to talk with resolver (announce, resolve, heartbeat)
    ResolverServiceClient resolv_cli_;

    //! The logger of this node
    logger::LogInterface *p_logger_;

private:
    //! Name of this node as it has been assigned by resolver
    std::string name_;

    //! State of this node
    State state_;

    //! Id of the thread in which this node has been created
    boost::thread::id thread_id_;

    //! Heartbeat thread
    boost::thread heartbeat_thread_;

    //! List of sockets
    std::set<socket::Socket*> sockets_;

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

    //! Signal (SIGINT) handler
    static void signalHandler(int s);

    //! Routine to set up the signal (SIGINT) handler
    static void setupSIGINTHandler();

public:
    friend class socket::Socket;
};

} // namespace b0

#endif // B0__NODE_H__INCLUDED

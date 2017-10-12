#ifndef NODE_H_INCLUDED
#define NODE_H_INCLUDED

#include "resolver.pb.h"
#include <b0/protobufhelpers.hpp>
#include <b0/logger/logger.h>

#include <string>
#include <boost/thread.hpp>
#include <boost/format.hpp>

namespace b0
{

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
class Node : public LogInterface
{
public:
    using LogInterface::log;

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
    void spinOnce();

    /*!
     * \brief Run the spin loop (continuously call spinOnce(), at the specified rate)
     *
     * \param spinRate the approximate frequency (in Hz) at which spinOnce() will be called
     */
    void spin(double spinRate = 10.0);

protected:
    /*!
     * \brief Connect to resolver node
     */
    virtual void connectToResolver();

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
    void log(b0::logger_msgs::LogLevel level, std::string message) override;

public:
    /*!
     * \brief Get the name assigned by resolver to this node
     */
    std::string getName() const;

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
     * Register a publisher for this node. Do not call this directly. Called by Publisher class.
     */
    void addPublisher(AbstractPublisher *pub);

    /*!
     * Register a publisher for this node. Do not call this directly. Called by Publisher class.
     */
    void removePublisher(AbstractPublisher *pub);

    /*!
     * Register a subscriber for this node. Do not call this directly. Called by Subscriber class.
     */
    void addSubscriber(AbstractSubscriber *sub);

    /*!
     * Unregister a subscriber for this node. Do not call this directly. Called by Subscriber class.
     */
    void removeSubscriber(AbstractSubscriber *sub);

    /*!
     * Register a service client for this node. Do not call this directly. Called by ServiceClient class.
     */
    void addServiceClient(AbstractServiceClient *cli);

    /*!
     * Unregister a service client for this node. Do not call this directly. Called by ServiceClient class.
     */
    void removeServiceClient(AbstractServiceClient *cli);

    /*!
     * Register a service server for this node. Do not call this directly. Called by ServiceServer class.
     */
    void addServiceServer(AbstractServiceServer *srv);

    /*!
     * Unregister a service server for this node. Do not call this directly. Called by ServiceServer class.
     */
    void removeServiceServer(AbstractServiceServer *srv);

public:
    /*!
     * \brief Fills the NodeID message with info on how tu uniquely identify this node in the network.
     */
    virtual void getNodeID(b0::resolver_msgs::NodeID &node_id);

    /*!
     * \brief Return the public address (IP or hostname) to reach this node on the network
     */
    virtual std::string hostname();

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
     * \brief Get the socket connected to resolver service
     */
    zmq::socket_t & resolverSocket() {return resolv_socket_;}

protected:
    //! ZeroMQ Context used by all sockets of this node
    zmq::context_t context_;

    //! ZeroMQ REQ socket used to talk with resolver (announce, resolve, heartbeat)
    zmq::socket_t resolv_socket_;

    //! The logger of this node
    Logger<Node> logger_;

private:
    //! Name of this node as it has been assigned by resolver
    std::string name_;

    //! Id of the thread in which this node has been created
    boost::thread::id thread_id_;

    //! Heartbeat thread
    boost::thread heartbeat_thread_;

    //! List of publishers
    std::set<AbstractPublisher*> publishers_;

    //! List of subscribers
    std::set<AbstractSubscriber*> subscribers_;

    //! List of service clients
    std::set<AbstractServiceClient*> service_clients_;

    //! List of service servers
    std::set<AbstractServiceServer*> service_servers_;

    //! Address of the proxy's XSUB socket
    std::string xsub_sock_addr_;

    //! Address of the proxy's XPUB socket
    std::string xpub_sock_addr_;

    static std::atomic<bool> quit_flag_;

    bool shutdown_flag_;

    static bool sigint_handler_setup_;

    static void signalHandler(int s);

    static void setupSIGINTHandler();

public:
    friend class AbstractPublisher;
    friend class AbstractSubscriber;
    friend class AbstractServiceClient;
    friend class AbstractServiceServer;
};

} // namespace b0

#endif // NODE_H_INCLUDED

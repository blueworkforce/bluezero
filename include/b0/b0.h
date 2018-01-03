#include <b0/node.h>
#include <b0/publisher.h>
#include <b0/subscriber.h>
#include <b0/service_client.h>
#include <b0/service_server.h>
#include <b0/exceptions.h>

/*!
 * \mainpage BlueWorkforce Middleware - A brief description
 *
 * \section api API Design
 *
 * The main class used to create a node is b0::Node. Node uses two-phase initialization, so you must call b0::Node::init() after the constructor, and b0::Node::cleanup() before the destructor.
 *
 * Also, b0::Node::spinOnce() must be called periodically to process incoming messages (or just call b0::Node::spin() once).
 *
 * \image html node-state-machine.png Node state transtion diagram.
 *
 * b0::Node::init() will initialize the node and announce its name to the
 * resolver node, and it will initialize each of its publishers, subscribers,
 * clients and servers.
 *
 * Any publishers, subscribers, service client and servers must be constructed prior to calling b0::Node::init().
 *
 * NOTE: In a thread there should be at most one b0::Node.
 *
 * The two ways of interconnecting nodes are:
 * - \b services, like clients and servers (functionality is provided by classes b0::ServiceClient and b0::ServiceServer)
 * - \b topics, like in publishers and subscribers (functionality provided by classes b0::Publisher and b0::Subscriber)
 *
 *
 * \section resolver_intro Resolver node
 *
 * The most important part of the network is the resolver node.
 * It is implemented in b0::resolver::Resolver and will provide following services to other nodes:
 *
 * - node name resolution
 * - socket name resolution
 * - a global XPUB/XSUB proxy
 * - liveness monitoring
 * - tracking of connected nodes
 * - clock synchronization (see \ref timesync)
 *
 * \b Important: you must have the resolver node running prior to running any node. See \ref remote_nodes for more information about running distributed nodes.
 *
 * \section examples Examples
 *
 * \subsection example_pubsub Topics (Publisher/Subscriber)
 *
 * Example of how to create a simple node with one publisher and sending
 * some messages to some topic:
 *
 * \include examples/publisher_subscriber/publisher_node.cpp
 *
 * And the corresponding example of a simple node with one subscriber:
 *
 * \include examples/publisher_subscriber/subscriber_node.cpp
 *
 * You can have multiple publishers and subscribers as well:
 *
 * \ref multi_publisher.cpp "Node with multiple publishers"
 *
 * \ref multi_subscriber.cpp "Node with multiple subscribers"
 *
 * And following is an example of using it in a more object-oriented way:
 *
 * \ref publisher_node_object.cpp "OOP publisher node"
 *
 * \ref subscriber_node_object.cpp "OOP subscriber node"
 *
 * \subsection example_clisrv Services (Client/Server)
 *
 * Example of how to create a simple node with a service client:
 *
 * \include client_node.cpp
 *
 * And the corresponding example of a simple node with a service server:
 *
 * \include server_node.cpp
 *
 * And the same thing, object-oriented:
 *
 * \ref client_node_object.cpp "OOP client node"
 *
 * \ref server_node_object.cpp "OOP server node"
 *
 *
 * \page remote_nodes Connecting remote nodes
 *
 * When using nodes distributed across multiple machines, you must make sure that
 * all nodes are able to reach each other, using IP and TCP protocol.
 *
 * \section Example
 *
 * Suppose we have a network with two machines: A and B.
 *
 * - Machine A: 192.168.1.5 will run the resolver node and a subscriber node.
 * - Machine B: 192.168.1.6 will run a publisher node.
 *
 * By default, nodes will use their hostnames when announcing socket addresses.
 * If it is not possible to reach a machine via its hostname externally (and in general,
 * it is not), IP addresses must be provided instead, by using environment variables.
 *
 * The two varables to specify are BWF_HOST_ID and BWF_RESOLVER.
 *
 * **BWF_HOST_ID** it is used when announcing socket addresses to the resolver. It defaults to the
 * machine hostname.
 *
 * **BWF_RESOLVER** is used on all nodes except the resolver node to specify the address of
 * the resolver socket. It defaults to tcp://localhost:22000, so it is not required to specify
 * this variasble if everything is running on the same machine.
 *
 * When starting the resolver node, BWF_RESOLVER_PORT can be used to specify a different TCP
 * port number to listen to.
 *
 * \subsection remote_a_resolver Machine A - starting resolver
 *
 * On machine A we run
 *
 * ~~~
 * export BWF_HOST_ID=192.168.1.5
 *
 * ./resolver
 * ~~~
 *
 * to run the resolver node.
 *
 * \subsection remote_a_subscriber Machine A - starting subscriber
 *
 * On machine A we run
 *
 * ~~~
 * export BWF_HOST_ID=192.168.1.5
 *
 * ./examples/publisher_subscriber/subscriber_node
 * ~~~
 *
 * to run the subscriber node.
 *
 * Note that we don't need to specify BWF_RESOLVER, because the default value (tcp://localhost:22000)
 * is sufficient to reach the resolver which is running on the same machine.
 *
 * \subsection remote_b_publisher Machine B - starting publisher
 *
 * On machine B we run
 *
 * ~~~
 * export BWF_HOST_ID=192.168.1.6
 *
 * export BWF_RESOLVER=tcp://192.168.1.5:22000
 *
 * ./examples/publisher_subscriber/publisher_node
 * ~~~
 *
 * to run the publisher node.
 *
 */


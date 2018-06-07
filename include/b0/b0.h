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
 * The main class used to create a node is b0::Node. Node uses two-phase initialization, so you must call b0::Node::init() after the constructor, and b0::Node::cleanup() before the destructor. <b>Do not call b0::Node::init() from your node class constructor!</b>
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
 * The two ways of interconnecting nodes are:
 * - \b services, like clients and servers (functionality is provided by classes b0::ServiceClient and b0::ServiceServer)
 * - \b topics, like in publishers and subscribers (functionality provided by classes b0::Publisher and b0::Subscriber)
 *
 * \section threading Threading and thread safety
 *
 * The functions of the library are not thread-safe, and so are the functions of ZeroMQ.
 * Thus, every node must be accessed always from the same thread.
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
 * When distributing nodes across multiple machines, you must make sure that:
 * - nodes know how to reach the resolver node;
 * - all nodes are able to reach each other, using IP and TCP protocol.
 *
 * \section reaching_resolver Reaching the resolver node
 *
 * Nodes by default will try to reach resolver at tcp://localhost:22000, which only works
 * when testing all nodes on the same machine. When the resolver node is on another machine,
 * (for example the machine hostname is resolver-node-host.local)
 * the environment variable B0_RESOLVER must be set to the correct address, e.g.:
 *
 * ~~~
 * export B0_RESOLVER="tcp://resolver-node-host.local:22000"
 * ~~~
 *
 * prior to launching every node, or in .bashrc or similar.
 *
 * When B0_RESOLVER is not specified it defaults to "tcp://localhost:22000".
 *
 * \section reaching_nodes Reaching every other node
 *
 * Nodes also need to be able to reach each other node.
 * When a node creates a directly addressed socket, such as ServiceClient or ServiceServer, it
 * will advertise that socket name and address to resolver.
 *
 * Since there is no reliable way of automatically determining the correct IP address of the node
 * (as there may be more than one), by default the node will use its hostname when specifying the
 * socket TCP address.
 *
 * This requires that all machines are able to reach each other by their hostnames.
 * This is the case when there is a name resolution service behind, such as a DNS, or
 * Avahi/ZeroConf/Bonjour.
 *
 * Suppose we have a network with two machines: A and B.
 * Machine A hostname is alice.local, and machine B hostname is bob.local.
 *
 * If from machine A we are able to reach (ping) machine B by using its hostname bob.local,
 * and vice-versa, from machine B we are able to reach machine A by using its hostname alice.local,
 * there is no additional configuration to set. Otherwise, we need to explicitly tell how a machine
 * is reached from outside (i.e. what's the correct IP or hostname), by setting the B0_HOST_ID
 * environment variable, e.g.:
 *
 * ~~~
 * export B0_HOST_ID="192.168.1.3"
 * ~~~
 *
 * When B0_HOST_ID is not specified it defaults to the machine hostname.
 *
 * \section remote_nodes_example Example
 *
 * Suppose we have a network with two machines: A and B.
 *
 * - Machine A (192.168.1.5) will run the resolver node and a subscriber node.
 * - Machine B (192.168.1.6) will run a publisher node.
 *
 * By default, nodes will use their hostnames when announcing socket addresses.
 * We override this behavior, by setting B0_HOST_ID to the machine IP address.
 *
 * \subsection remote_nodes_example_a_resolver Machine A - starting resolver
 *
 * On machine A we run
 *
 * ~~~
 * export B0_HOST_ID="192.168.1.5"
 *
 * ./resolver
 * ~~~
 *
 * to run the resolver node.
 *
 * \subsection remote_nodes_example_a_subscriber Machine A - starting subscriber
 *
 * On machine A we run
 *
 * ~~~
 * export B0_HOST_ID="192.168.1.5"
 *
 * ./examples/publisher_subscriber/subscriber_node
 * ~~~
 *
 * to run the subscriber node.
 *
 * Note that for this machine we don't need to specify B0_RESOLVER, because the default value
 * (tcp://localhost:22000) is good to reach the resolver socket.
 *
 * \subsection remote_nodes_example_b_publisher Machine B - starting publisher
 *
 * On machine B we run
 *
 * ~~~
 * export B0_HOST_ID="192.168.1.6"
 *
 * export B0_RESOLVER="tcp://192.168.1.5:22000"
 *
 * ./examples/publisher_subscriber/publisher_node
 * ~~~
 *
 * to run the publisher node.
 *
 */


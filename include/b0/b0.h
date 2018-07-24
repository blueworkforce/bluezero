#ifndef B0__B0_H__INCLUDED
#define B0__B0_H__INCLUDED

#include <b0/config.h>

/*!
 * \mainpage BlueWorkforce Middleware - A brief description
 *
 * \section api API Design
 *
 * The main class used to create a node is b0::Node.
 *
 * A node implements one part of the \ref protocol "protocol", with the other counterpart of the protocol implemented by the \ref resolver_intro "resolver node".
 *
 * Node uses two-phase initialization, so you must call b0::Node::init() after the constructor, and b0::Node::cleanup() before the destructor. <b>Do not call b0::Node::init() from your node class constructor!</b>
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
 *
 * The resolver node implements a part of the \ref protocol "protocol", with the other counterpart of the protocol implemented by the \ref b0::Node "node".
 *
 * The resolver node is implemented in b0::resolver::Resolver and will provide following services to other nodes:
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
 * \ref publisher_subscriber_multi/multi_publisher.cpp "Node with multiple publishers"
 *
 * \ref publisher_subscriber_multi/multi_subscriber.cpp "Node with multiple subscribers"
 *
 * And following is an example of using it in a more object-oriented way:
 *
 * \ref publisher_subscriber_oop/publisher_node_object.cpp "OOP publisher node"
 *
 * \ref publisher_subscriber_oop/subscriber_node_object.cpp "OOP subscriber node"
 *
 * \subsection example_clisrv Services (Client/Server)
 *
 * Example of how to create a simple node with a service client:
 *
 * \include examples/client_server/client_node.cpp
 *
 * And the corresponding example of a simple node with a service server:
 *
 * \include examples/client_server/server_node.cpp
 *
 * And the same thing, object-oriented:
 *
 * \ref client_server_oop/client_node_object.cpp "OOP client node"
 *
 * \ref client_server_oop/server_node_object.cpp "OOP server node"
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
 * When a node creates a directly addressed socket, such as b0::ServiceClient or b0::ServiceServer, it
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
 *
 * \page protocol Protocol
 *
 * This page describes the BlueZero protocol. Knowledge of the communication protocol is
 * not required to use the BlueZero library, but serves as a specification for
 * re-implementing BlueZero.
 *
 * \section protocol_intro Introduction
 *
 * The transport of messages exchanged between BlueZero nodes and the resolver node is
 * implemented with ZeroMQ.
 *
 * The messages are defined using Google Protocol Buffers.
 *
 * Message payloads (used by BlueZero sockets) are wrapped in a b0::message::MessageEnvelope message.
 *
 * The network architecture is mostly centralized: every node will talk to the resolver node, except
 * for services which use dedicated sockets, and topics which use a XPUB/XSUB proxy.
 *
 * The resolver node offers one service ('resolv'), and also runs the XPUB/XSUB proxy.
 *
 * There are three main phases of the lifetime of a node:
 * - startup
 * - normal lifetime
 * - shutdown
 *
 * \section node_startup Node startup
 *
 * In the startup phase a node must announce its presence to the resolver node via the
 * b0::message::AnnounceNodeRequest message.
 * The resolver will reply with the b0::message::AnnounceNodeResponse message,
 * containing the final node name (as it may be changed in case of a
 * name clash) and important info for node communication, such as the XPUB/XSUB addresses.
 *
 * \mscfile node-startup.msc
 *
 * \subsection node_startup_topics Topics
 *
 * As part of the \ref graph "node graph protocol", if the node subscribes or publishes on some topic,
 * it will inform the resolver node via the b0::message::NodeTopicRequest message.
 *
 * \mscfile graph-topic.msc
 *
 * \subsection node_startup_services Services
 *
 * If the node offers some service, it will announce each service name and address
 * via the b0::message::AnnounceServiceRequest message.
 *
 * \mscfile node-startup-service.msc
 *
 * Additionally, as part of the \ref graph "node graph protocol", if the node offers or uses some service,
 * it will inform the resolver node via the b0::message::NodeServiceRequest message.
 *
 * \mscfile graph-service.msc
 *
 * \section node_lifetime Normal node lifetime
 *
 * During node lifetime, a node will periodically send a heartbeat to allow the resolver node
 * to track dead nodes.
 *
 * \mscfile node-lifetime.msc
 *
 * \subsection node_lifetime_topics Topics
 *
 * When a node wants to publish to some topic, it has to use the XPUB address given by resolver
 * in the b0::message::AnnounceNodeResponse message.
 * The payload to write to the socket is a b0::message::MessageEnvelope message.
 *
 * \mscfile topic-write.msc
 *
 * Similarly, when it wants to subscribe to some topic, the messages are read from the XSUB
 * socket.
 *
 * \mscfile topic-read.msc
 *
 * \subsection node_lifetime_services Services
 *
 * When a node wants to use a service, it has to resolve the service name to an address,
 * via the b0::message::ResolveServiceRequest message.
 *
 * \mscfile service-resolve.msc
 *
 * The request payload to write to the socket, as well as the response payload to be read
 * from the socket, are a b0::message::MessageEnvelope message.
 *
 * \mscfile service-call.msc
 *
 * \section node_shutdown Node shutdown
 *
 * When a node is shutdown, it will send a b0::message::ShutdownNodeRequest message to inform the resolver node about that.
 *
 * \mscfile node-shutdown.msc
 *
 * Additionally, it will send b0::message::NodeTopicRequest and b0::message::NodeServiceRequest to inform about not using or offering the
 * topics or services anymore.
 *
 * \mscfile graph-topic.msc
 * \mscfile graph-service.msc
 *
 *
 * \page graph Graph protocol
 *
 * The graph protocol is a subset of the \ref protocol "protocol", consisting of a series of messages used to allow introspection of node, topics, and services connections.
 *
 * The messages sent by sockets to inform resolver about these connections are b0::message::NodeTopicRequest and b0::message::NodeServiceRequest (see \ref protocol).
 *
 * Additionally, the b0::message::GetGraphRequest message can be used to retrieve the graph:
 *
 * \mscfile graph-get.msc
 *
 * The program b0_graph_console (and also gui/b0_graph_console_gui) included in BlueZero is
 * an example of displaying such graph, while whatching for changes to it in realtime.
 *
 * Here is a rendering of the graph (b0::message::Graph) during a BlueZero session
 * with several nodes running:
 *
 * \dotfile graph-example.gv
 *
 * Black rectangles are nodes, red diamonds are services, and blue ovals are topics.
 *
 * An arrow from node to topic means a node is publishing to a topic. Vice-versa, an arrow from topic to node means a node is subscribing to a topic.
 *
 * An arrow from node to service means a node is offering a service. Vice-versa, an arrow from service to node means a node is using a service.
 *
 * Nodes have an implicit connection to the 'resolv' service, however it is not shown in the graph.
 */

#ifndef B0_EXPORT
#ifdef _WIN32
#ifdef B0_LIBRARY
#define B0_EXPORT __declspec(dllexport)
#else // B0_LIBRARY
#define B0_EXPORT __declspec(dllimport)
#endif // B0_LIBRARY
#else // _WIN32
#define B0_EXPORT
#endif // _WIN32
#endif // B0_EXPORT

#endif // B0__B0_H__INCLUDED

#include <b0/node.h>
#include <b0/service_server.h>
#include <b0/message/heartbeat_request.h>
#include <b0/message/heartbeat_response.h>

#include <iostream>

/*! \example server_node.cpp
 * This is an example of a simple node with a service server.
 */

//! \cond HIDDEN_SYMBOLS

/*
 * This callback will be called whenever a request message is read from the socket
 */
void callback(const b0::message::HeartbeatRequest &req, b0::message::HeartbeatResponse &rep)
{
    //std::cout << "Received: " << req << std::endl;
    rep.ok = false;
    rep.time_usec = 12345;
    //std::cout << "Sending: " << rep << std::endl;
}

int main(int argc, char **argv)
{
    /*
     * Create a node named "server"
     *
     * Note: if another node with the same name exists on the network, this node will
     *       get a different name
     */
    b0::Node node("server");

    /*
     * Create a ServiceServer for a service named "control"
     * It will call the specified callback upon receiving requests.
     */
    b0::ServiceServer srv(&node, "control", b0::ServiceServer::CallbackTypedMsg(), &callback);

    /*
     * Initialize the node (will announce node name to the network, and do other nice things)
     */
    node.init();

    /*
     * Spin the node (continuously process incoming requests and call callbacks)
     */
    node.spin();

    /*
     * Perform cleanup (stop threads, notify resolver that this node has quit, ...)
     */
    node.cleanup();

    return 0;
}

//! \endcond


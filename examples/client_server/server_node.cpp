#include <b0/node.h>
#include <b0/service_server.h>
#include "client_server.pb.h"

#include <iostream>

/*! \example server_node.cpp
 * This is an example of a simple node with a service server.
 */

//! \cond HIDDEN_SYMBOLS

/*
 * This callback will be called whenever a request message is read from the socket
 */
void callback(const example_msgs::TestRequest &req, example_msgs::TestResponse &resp)
{
    std::cout << "Received:" << std::endl << req.DebugString() << std::endl;

    resp.set_sum(req.a() + req.b());

    std::cout << "Sending:" << std::endl << resp.DebugString() << std::endl;
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
    b0::ServiceServer<example_msgs::TestRequest, example_msgs::TestResponse> srv(&node, "control", &callback);

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


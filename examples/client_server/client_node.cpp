#include <b0/node.h>
#include <b0/service_client.h>
#include "client_server.pb.h"

#include <iostream>

/*! \example client_node.cpp
 * This is an example of a simple node with a service client
 */

//! \cond HIDDEN_SYMBOLS

int main(int argc, char **argv)
{
    /*
     * Create a node named "client"
     */
    b0::Node node("client");

    /*
     * Create a ServiceClient that will connect to the service "control"
     */
    b0::ServiceClient<example_msgs::TestRequest, example_msgs::TestResponse> cli(&node, "control");

    /*
     * Initialize the node (will announce the node name to the network, and do other nice things)
     */
    node.init();

    /*
     * Create a TestRequest message and set its fields
     */
    example_msgs::TestRequest req;
    req.set_a(100);
    req.set_b(35);
    std::cout << "Sending:" << std::endl << req.DebugString() << std::endl;

    /*
     * The response will be written here
     */
    example_msgs::TestResponse resp;

    /*
     * Call the service (blocking)
     */
    cli.call(req, resp);
    std::cout << "Received:" << std::endl << resp.DebugString() << std::endl;

    return 0;
}

//! \endcond


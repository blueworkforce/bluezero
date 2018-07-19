#include <b0/node.h>
#include <b0/publisher.h>
#include <b0/message/log_entry.h>

#include <iostream>

/*! \example publisher_node.cpp
 * This is an example of creating a simple node with one publisher
 */

//! \cond HIDDEN_SYMBOLS

int main(int argc, char **argv)
{
    /*
     * Create a node named "publisher"
     */
    b0::Node node("publisher");

    /*
     * Create a Publisher to publish on topic "A"
     */
    b0::Publisher pub(&node, "A");

    /*
     * Initialize the node (will announce node name to the network, and do other nice things)
     */
    node.init();

    int i = 0;
    while(!node.shutdownRequested())
    {
        /*
         * Process messages from node's sockets
         */
        node.spinOnce();

        /*
         * Create a message to send
         */
        b0::message::LogEntry e;
        e.message = "Hello, world!";

        /*
         * Send the message on the "A" topic
         */
        std::cout << "Sending: " << e.message << std::endl;
        pub.publish(e);

        /*
         * Wait some time
         */
        boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
    }

    /*
     * Perform cleanup (stop threads, notify resolver that this node has quit, ...)
     */
    node.cleanup();

    return 0;
}

//! \endcond


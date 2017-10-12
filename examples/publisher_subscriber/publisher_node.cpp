#include <b0/node.h>
#include <b0/publisher.h>
#include "publisher_subscriber.pb.h"

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
    b0::Publisher<example_msgs::TestMessage> pub(&node, "A");

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
        example_msgs::TestMessage msg;
        msg.set_data(i++);

        /*
         * Send the message on the "A" topic
         */
        std::cout << "Sending:" << std::endl << msg.DebugString() << std::endl;
        pub.publish(msg);

        /*
         * Wait some time
         */
        boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
    }
    return 0;
}

//! \endcond


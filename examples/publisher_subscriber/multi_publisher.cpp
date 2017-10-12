#include <b0/node.h>
#include <b0/publisher.h>
#include "publisher_subscriber.pb.h"

#include <iostream>

/*! \example multi_publisher.cpp
 * This is an example of having multiple publishers in one node
 */

//! \cond HIDDEN_SYMBOLS

int main(int argc, char **argv)
{
    /*
     * Create a node named "publisher"
     */
    b0::Node node("publisher");

    /*
     * Create two publishers:
     *  - pubA publishes messages of type example_msgs::TestMessage on topic "A"
     *  - pubB publishes messages of type example_msgs::TestMessage2 on topic "B"
     */
    b0::Publisher<example_msgs::TestMessage> pubA(&node, "A");
    b0::Publisher<example_msgs::TestMessage2> pubB(&node, "B");

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
         * Publish some data on "A"
         */
        example_msgs::TestMessage msg1;
        msg1.set_data(i++);
        pubA.publish(msg1);

        /*
         * Publish some data on "B"
         */
        example_msgs::TestMessage2 msg2;
        msg2.set_data((boost::format("string_%d") % i++).str());
        pubB.publish(msg2);

        /*
         * Wait some time (simulate I/O wait...)
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


#include <b0/node.h>
#include <b0/subscriber.h>
#include "publisher_subscriber.pb.h"

#include <iostream>

/*! \example subscriber_node.cpp
 * This is an example of a simple node with one callback-based subscriber
 */

//! \cond HIDDEN_SYMBOLS

/*
 * This callback will be called whenever a message is received on any
 * of the subscribed topics
 */
void callback(std::string topic, const example_msgs::TestMessage &msg)
{
    std::cout << "Received (on topic '" << topic << "'):" << std::endl
        << msg.DebugString() << std::endl;
}

int main(int argc, char **argv)
{
    /*
     * Create a node named "subscriber"
     */
    b0::Node node("subscriber");

    /*
     * Create a Subscriber to subscribe to topic "A"
     * It will call the specified callback upon receiving messages
     */
    b0::Subscriber<example_msgs::TestMessage> sub(&node, "A", &callback);

    /*
     * Initialize the node (will announce node name to the network, and do other nice things)
     */
    node.init();

    /*
     * Spin the node (continuously process incoming messages and call callbacks)
     */
    node.spin();

    return 0;
}

//! \endcond


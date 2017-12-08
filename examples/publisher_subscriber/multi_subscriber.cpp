#include <b0/node.h>
#include <b0/subscriber.h>
#include "publisher_subscriber.pb.h"

#include <iostream>

/*! \example multi_subscriber.cpp
 * This is an example of having multiple subscribers inside one node
 */

//! \cond HIDDEN_SYMBOLS

void callback1(const example_msgs::TestMessage &msg)
{
    std::cout << "Received:" << std::endl
        << msg.DebugString() << std::endl;
}

void callback2(const example_msgs::TestMessage2 &msg)
{
    std::cout << "Received:" << std::endl
        << msg.DebugString() << std::endl;
}

int main(int argc, char **argv)
{
    /*
     * Create a node named "subscriber"
     */
    b0::Node node("subscriber");

    /*
     * Subscribe on topic "A" for messages of type example_msgs::TestMessage
     * and call callback1(std::string topic, const example_msgs::TestMessage &msg)
     * when a message is received.
     */
    b0::Subscriber<example_msgs::TestMessage> subA(&node, "A", &callback1);

    /*
     * Similar as above for topic "B" and type example_msgs::TestMessage2
     */
    b0::Subscriber<example_msgs::TestMessage2> subB(&node, "B", &callback2);

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


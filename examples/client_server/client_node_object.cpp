#include <b0/node.h>
#include <b0/service_client.h>
#include "client_server.pb.h"

#include <iostream>

/*! \example client_node_object.cpp
 * This is an example of creating a node with a service client by subclassing b0::Node
 */

//! \cond HIDDEN_SYMBOLS

class TestClientNode : public b0::Node
{
public:
    TestClientNode()
        : cli_(this, "control")
    {
    }

    void run()
    {
        example_msgs::TestRequest req;
        req.set_a(100);
        req.set_b(35);

        example_msgs::TestResponse resp;

        std::cout << "Sending:" << std::endl << req.DebugString() << std::endl;

        cli_.call(req, resp);

        std::cout << "Received:" << std::endl << resp.DebugString() << std::endl;
    }

protected:
    b0::ServiceClient<example_msgs::TestRequest, example_msgs::TestResponse> cli_;
};

int main(int argc, char **argv)
{
    TestClientNode node;
    node.init();
    node.run();
    return 0;
}

//! \endcond


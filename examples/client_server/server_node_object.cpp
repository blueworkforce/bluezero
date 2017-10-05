#include <b0/node.h>
#include <b0/service_server.h>
#include "client_server.pb.h"

#include <iostream>

/*! \example server_node_object.cpp
 * This is an example of creating a node with a service server by subclassing b0::Node
 */

//! \cond HIDDEN_SYMBOLS

class TestServerNode : public b0::Node
{
public:
    TestServerNode()
        : Node("server"),
          srv_(this, "control", &TestServerNode::on)
    {
    }

    void on(const example_msgs::TestRequest &req, example_msgs::TestResponse &resp)
    {
        std::cout << "Received:" << std::endl << req.DebugString() << std::endl;
        resp.set_sum(req.a() + req.b());
        std::cout << "Sending:" << std::endl << resp.DebugString() << std::endl;
    }

protected:
    b0::ServiceServer<example_msgs::TestRequest, example_msgs::TestResponse> srv_;
};

int main(int argc, char **argv)
{
    TestServerNode node;
    node.init();
    node.spin();
    return 0;
}

//! \endcond


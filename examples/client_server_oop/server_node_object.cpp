#include <b0/node.h>
#include <b0/service_server.h>

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
          srv_(this, "control", boost::bind(&TestServerNode::on, this, _1, _2))
    {
    }

    void on(const std::string &req, std::string &rep)
    {
        std::cout << "Received: " << req << std::endl;
        rep = "hi";
        std::cout << "Sending: " << rep << std::endl;
    }

protected:
    b0::ServiceServer srv_;
};

int main(int argc, char **argv)
{
    TestServerNode node;
    node.init();
    node.spin();
    node.cleanup();
    return 0;
}

//! \endcond


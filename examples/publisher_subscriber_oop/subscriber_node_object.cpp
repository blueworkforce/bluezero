#include <b0/node.h>
#include <b0/subscriber.h>

#include <iostream>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

/*! \example publisher_subscriber_oop/subscriber_node_object.cpp
 * This is an example of creating a node by subclassing b0::Node.
 * Useful for overriding some node's behavior.
 */

//! \cond HIDDEN_SYMBOLS

class TestSubscriberNode : public b0::Node
{
public:
    TestSubscriberNode(std::string topic)
        : Node("subscriber"),
          sub_(this, topic, &TestSubscriberNode::on, this)
    {
    }

    void on(const std::string &msg)
    {
        std::cout << "Received: " << msg << std::endl;
    }

private:
    b0::Subscriber sub_;
};

int main(int argc, char **argv)
{
    b0::addOptions()
        ("topic", po::value<std::string>()->default_value("A"), "topic name")
    ;
    b0::init(argc, argv);
    TestSubscriberNode node(b0::getOption("topic").as<std::string>());
    node.init();
    node.spin();
    node.cleanup();
    return 0;
}

//! \endcond


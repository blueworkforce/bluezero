#include <iostream>

#include <boost/program_options.hpp>

#include <b0/node.h>
#include <b0/subscriber.h>

namespace po = boost::program_options;

void callback(const std::string &payload)
{
    std::cout << payload << std::flush;
}

int main(int argc, char **argv)
{
    std::string node_name = "b0_topic_echo", topic_name = "";
    b0::addOptions()
        ("node-name,n", po::value<std::string>(&node_name), "name of node")
        ("topic-name,t", po::value<std::string>(&topic_name), "name of topic")
    ;
    b0::addPositionalOption("topic-name");
    b0::init(argc, argv);

    b0::Node node(node_name);
    b0::Subscriber sub(&node, topic_name, callback);
    node.init();
    node.spin();
    node.cleanup();
    return 0;
}


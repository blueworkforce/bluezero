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
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "display help message")
        ("node-name,n", po::value<std::string>(&node_name), "name of node")
        ("topic-name,t", po::value<std::string>(&topic_name), "name of topic")
    ;
    po::variables_map vm;
    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);
    }
    catch(po::unknown_option &ex)
    {
        std::cerr << ex.what() << std::endl << desc << std::endl;
        return 1;
    }
    po::notify(vm);

    if(vm.count("help"))
    {
        std::cout << desc << std::endl;
        return 0;
    }

    b0::Node node(node_name);
    b0::Subscriber sub(&node, topic_name, callback);
    node.init();
    node.spin();
    node.cleanup();
    return 0;
}


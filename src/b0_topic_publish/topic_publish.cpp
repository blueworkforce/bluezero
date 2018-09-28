#include <string>
#include <iostream>
#include <iterator>

#include <boost/thread.hpp>
#include <boost/program_options.hpp>

#include <b0/node.h>
#include <b0/publisher.h>

namespace po = boost::program_options;

int main(int argc, char **argv)
{
    std::string node_name = "b0_topic_publish", topic_name = "";
    double rate = 0.0;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "display help message")
        ("node-name,n", po::value<std::string>(&node_name), "name of node")
        ("topic-name,t", po::value<std::string>(&topic_name), "name of topic")
        ("rate,r", po::value<double>(&rate), "publish rate (0 means one-shot)")
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

    std::cin >> std::noskipws;
    std::istream_iterator<char> it(std::cin);
    std::istream_iterator<char> end;
    std::string payload(it, end);

    b0::Node node(node_name);
    b0::Publisher pub(&node, topic_name);
    node.init();
    while(!node.shutdownRequested())
    {
        pub.publish(payload);
        if(rate == 0.0) break;
        boost::this_thread::sleep_for(boost::chrono::microseconds{long(1000000 / rate)});
    }
    node.cleanup();
    return 0;
}


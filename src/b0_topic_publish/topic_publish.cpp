#include <string>
#include <iostream>
#include <iterator>

#include <boost/program_options.hpp>
#include <boost/thread.hpp>

#include <b0/node.h>
#include <b0/publisher.h>

namespace po = boost::program_options;

int main(int argc, char **argv)
{
    std::string node_name = "b0_topic_publish", topic_name = "", content_type = "";
    double rate = 0.0;
    b0::addOptions()
        ("node-name,n", po::value<std::string>(&node_name), "name of node")
        ("topic-name,t", po::value<std::string>(&topic_name), "name of topic")
        ("content-type,c", po::value<std::string>(&content_type), "content type")
        ("rate,r", po::value<double>(&rate), "publish rate (0 means one-shot)")
    ;
    b0::addPositionalOption("topic-name");
    b0::init(argc, argv);

    std::cin >> std::noskipws;
    std::istream_iterator<char> it(std::cin);
    std::istream_iterator<char> end;
    std::string payload(it, end);

    b0::Node node(node_name);
    b0::Publisher pub(&node, topic_name);
    node.init();
    while(!node.shutdownRequested())
    {
        pub.publish(content_type, payload);
        if(rate == 0.0) break;
        boost::this_thread::sleep_for(boost::chrono::microseconds{long(1000000 / rate)});
    }
    node.cleanup();
    return 0;
}


#include <string>
#include <iostream>
#include <iterator>

#include <boost/thread.hpp>

#include <b0/node.h>
#include <b0/publisher.h>

int main(int argc, char **argv)
{
    std::string node_name = "b0_topic_publish", topic_name = "", content_type = "";
    double rate = 0.0;
    b0::addOptionString("node-name,n", "name of node", &node_name);
    b0::addOptionString("topic-name,t", "name of topic", &topic_name);
    b0::addOptionString("content-type,c", "content type", &content_type);
    b0::addOptionDouble("rate,r", "publish rate (0 means one-shot)", &rate);
    b0::setPositionalOption("topic-name");
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


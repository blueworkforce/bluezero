#include <string>
#include <iostream>
#include <iterator>

#include <b0/node.h>
#include <b0/publisher.h>

#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

int usage(int exit_code, const std::string &arg0)
{
    std::cout << "usage: " << arg0 << " [ -r <rate> ] <topic>" << std::endl;
    return exit_code;
}

int main(int argc, char **argv)
{
    double rate = 0.0;
    std::string topic;

    if(argc > 1 && std::string(argv[1]) == "-r")
    {
        if(argc != 4)
            return usage(1, argv[0]);
        rate = boost::lexical_cast<double>(argv[2]);
        topic = argv[3];
    }
    else if(argc == 2)
    {
        topic = argv[1];
    }
    else return usage(1, argv[0]);

    std::cin >> std::noskipws;
    std::istream_iterator<char> it(std::cin);
    std::istream_iterator<char> end;
    std::string payload(it, end);

    b0::Node node("b0_topic_publish");
    b0::Publisher pub(&node, topic);
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


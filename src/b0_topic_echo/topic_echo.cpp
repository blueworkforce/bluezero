#include <iostream>
#include <b0/node.h>
#include <b0/subscriber.h>

int usage(int exit_code, const std::string &arg0)
{
    std::cout << "usage: " << arg0 << " <topic>" << std::endl;
    return exit_code;
}

void callback(const std::string &payload)
{
    std::cout << payload << std::flush;
}

int main(int argc, char **argv)
{
    if(argc != 2)
        return usage(1, argv[0]);

    b0::Node node("b0_topic_echo");
    b0::Subscriber sub(&node, argv[1], callback);
    node.init();
    node.spin();
    node.cleanup();
    return 0;
}


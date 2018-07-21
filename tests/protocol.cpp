#include <iostream>

#include <b0/messages.h>

bool verbose = true;

template<typename T>
void test(const T &msg)
{
    std::string serialized;
    serialize(msg, serialized);
    if(verbose)
        std::cout << msg.type() << ": serialized: " << serialized << std::endl;
    T msg2;
    parse(msg2, serialized);
    std::string reserialized;
    serialize(msg2, reserialized);
    if(verbose)
        std::cout << msg.type() << ": re-serialized: " << reserialized << std::endl;
    if(serialized != reserialized)
    {
        std::cerr << "Test for " << msg.type() << " failed" << std::endl;
        exit(1);
    }
}

int main(int argc, char **argv)
{
    {
        b0::message::AnnounceNodeRequest msg;
        msg.node_name = "foo";
        test(msg);
    }

    {
        b0::message::LogEntry msg;
        msg.node_name = "foo";
        msg.level = b0::message::LogLevel::warn;
        msg.message = "Hello \x01\xff world";
        msg.time_usec = (uint64_t(1) << 60) + 5978629785;
        test(msg);
    }

    {
        b0::message::Graph g1;
        g1.nodes.push_back("a");
        g1.nodes.push_back("b");
        g1.nodes.push_back("c");
        b0::message::GraphLink l1;
        l1.node_name = "a";
        l1.other_name = "t";
        l1.reversed = false;
        g1.node_topic.push_back(l1);
        b0::message::GraphLink l2;
        l2.node_name = "b";
        l2.other_name = "t";
        l2.reversed = true;
        g1.node_topic.push_back(l2);
        b0::message::GraphLink l3;
        l3.node_name = "c";
        l3.other_name = "t";
        l3.reversed = true;
        g1.node_topic.push_back(l3);
        test(g1);
    }

    return 0;
}


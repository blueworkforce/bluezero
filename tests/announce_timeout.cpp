#include <iostream>
#include <boost/thread.hpp>
#include <boost/program_options.hpp>

#include <b0/node.h>
#include <b0/exceptions.h>

namespace po = boost::program_options;

int timeout = -1;
int expect_read_error = 0;

void node_thread()
{
    b0::Node node("testnode");
    node.setAnnounceTimeout(timeout);
    try
    {
        node.init();
    }
    catch(b0::exception::SocketReadError &ex)
    {
        // a timeout during announce phase will result in a SocketReadError exception
        exit(expect_read_error ? 0 : 1);
    }
    // if we reach here, a resolver node is running (test deviation)
    exit(3);
}

void timeout_thread()
{
    boost::this_thread::sleep_for(boost::chrono::seconds{4});
    exit(expect_read_error ? 1 : 0);
}

int main(int argc, char **argv)
{
    b0::addOptions()
        ("timeout,t", po::value<int>(&timeout)->required(), "timeout for announce phase")
        ("expect-read-error,e", po::value<int>(&expect_read_error)->required(), "expect read error")
    ;
    b0::setPositionalOption("timeout");
    b0::setPositionalOption("expect-read-error");
    b0::init(argc, argv);

    boost::thread t0(&timeout_thread);
    boost::thread t1(&node_thread);
    t0.join();
}


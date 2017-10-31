#include <boost/thread.hpp>

#include "resolver.pb.h"
#include <b0/resolver/resolver.h>
#include <b0/node.h>
#include <b0/publisher.h>
#include <b0/subscriber.h>

void resolver_thread()
{
    b0::resolver::Resolver node;
    node.init();
    node.spin();
}

void pub_thread()
{
    b0::Node node("pub");
    b0::Publisher<std::string> pub(&node, "topic1");
    node.init();
    for(;;) pub.publish("hello");
}

void sub_thread()
{
    b0::Node node("sub");
    b0::Subscriber<std::string> sub(&node, "topic1");
    node.init();
    std::string topic, payload;
    sub.read(topic, payload);
    exit(payload == "hello" ? 0 : 1);
}

void timeout_thread()
{
    boost::this_thread::sleep_for(boost::chrono::seconds{4});
    exit(1);
}

int main(int argc, char **argv)
{
    boost::thread t0(&timeout_thread);
    boost::thread t1(&resolver_thread);
    boost::this_thread::sleep_for(boost::chrono::seconds{1});
    boost::thread t2(&sub_thread);
    boost::this_thread::sleep_for(boost::chrono::seconds{1});
    boost::thread t3(&pub_thread);
    t0.join();
}


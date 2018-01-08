#include <atomic>
#include <boost/thread.hpp>

#include "test_msgs.pb.h"
#include "resolver.pb.h"
#include <b0/resolver/resolver.h>
#include <b0/node.h>
#include <b0/publisher.h>
#include <b0/subscriber.h>

std::atomic<bool> quit;
int result;

void resolver_thread()
{
    b0::resolver::Resolver node;
    node.init();
    node.spin();
}

void pub_thread()
{
    b0::Node node("pub");
    b0::Publisher<test_msgs::Msg1> pub1(&node, "topic1");
    b0::Publisher<test_msgs::Msg1> pub2(&node, "topic2");
    b0::Publisher<test_msgs::Msg1> pub3(&node, "topic3");
    node.init();
    test_msgs::Msg1 m;
    m.set_data(123456);
    while(!quit.load())
    {
        pub1.publish(m);
        pub2.publish(m);
        pub3.publish(m);
    }
    node.cleanup();
    exit(0);
}

void sub_thread()
{
    b0::Node node("sub");
    b0::Subscriber<test_msgs::Msg1> sub1(&node, "topic1");
    b0::Subscriber<test_msgs::Msg1> sub2(&node, "topic2");
    b0::Subscriber<test_msgs::Msg1> sub3(&node, "topic3");
    node.init();
    test_msgs::Msg1 m1, m2, m3;
    sub1.read(m1);
    sub2.read(m2);
    sub3.read(m3);
    int e = 123456;
    if(m1.data() == e && m2.data() == e && m3.data() == e)
        result = 0;
    quit.store(true);
    node.cleanup();
}

void timeout_thread()
{
    boost::this_thread::sleep_for(boost::chrono::seconds{4});
    exit(1);
}

int main(int argc, char **argv)
{
    result = 1;
    quit.store(true);
    boost::thread t0(&timeout_thread);
    boost::thread t1(&resolver_thread);
    boost::this_thread::sleep_for(boost::chrono::seconds{1});
    boost::thread t2(&sub_thread);
    boost::this_thread::sleep_for(boost::chrono::seconds{1});
    boost::thread t3(&pub_thread);
    t0.join();
}


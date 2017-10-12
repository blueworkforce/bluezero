#include <boost/thread.hpp>

#include "test_msgs.pb.h"

#include <b0/resolver/resolver.h>
#include <b0/node.h>
#include <b0/service_client.h>
#include <b0/service_server.h>

void resolver_thread()
{
    b0::resolver::Resolver node;
    node.init();
    node.spin();
}

void cli_thread()
{
    b0::Node node("cli");
    b0::ServiceClient<test_msgs::Req, test_msgs::Resp> cli(&node, "service1");
    node.init();
    test_msgs::Req req;
    test_msgs::Resp resp;
    req.set_a(123456);
    req.set_b(1);
    cli.call(req, resp);
    node.log(b0::logger_msgs::INFO, "server response: %s", resp.DebugString());
    exit(resp.c() == 123457 ? 0 : 1);
}

void srv_thread()
{
    b0::Node node("srv");
    b0::ServiceServer<test_msgs::Req, test_msgs::Resp> srv(&node, "service1");
    node.init();
    test_msgs::Req req;
    test_msgs::Resp resp;
    if(srv.read(req))
        resp.set_c(req.a() + req.b());
    srv.write(resp);
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
    boost::thread t2(&srv_thread);
    boost::this_thread::sleep_for(boost::chrono::seconds{1});
    boost::thread t3(&cli_thread);
    t0.join();
}


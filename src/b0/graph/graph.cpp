#include <b0/graph.h>

namespace b0
{

namespace graph
{

void notifyTopic(b0::Node &node, std::string topic_name, bool reverse, bool active)
{
    zmq::socket_t &resolv_sock = node.resolverSocket();
    b0::resolver_msgs::Request req0;
    b0::resolver_msgs::NodeTopicRequest &req = *req0.mutable_node_topic();
    b0::resolver_msgs::Response resp0;
    req.set_node_name(node.getName());
    req.set_topic_name(topic_name);
    req.set_reverse(reverse);
    req.set_active(active);
    s_send(resolv_sock, req0);
    s_recv(resolv_sock, resp0);
}

void notifyService(b0::Node &node, std::string service_name, bool reverse, bool active)
{
    zmq::socket_t &resolv_sock = node.resolverSocket();
    b0::resolver_msgs::Request req0;
    b0::resolver_msgs::NodeServiceRequest &req = *req0.mutable_node_service();
    b0::resolver_msgs::Response resp0;
    req.set_node_name(node.getName());
    req.set_service_name(service_name);
    req.set_reverse(reverse);
    req.set_active(active);
    s_send(resolv_sock, req0);
    s_recv(resolv_sock, resp0);
}

} // namespace graph

} // namespace b0


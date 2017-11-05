#include <b0/graph/graph.h>
#include <b0/node.h>
#include <b0/service_client.h>

#include "resolver.pb.h"

namespace b0
{

namespace graph
{

void notifyTopic(b0::Node &node, std::string topic_name, bool reverse, bool active)
{
    b0::Node::ResolverServiceClient &resolv_cli = node.resolverClient();
    b0::resolver_msgs::Request req0;
    b0::resolver_msgs::NodeTopicRequest &req = *req0.mutable_node_topic();
    b0::resolver_msgs::Response resp0;
    req.set_node_name(node.getName());
    req.set_topic_name(topic_name);
    req.set_reverse(reverse);
    req.set_active(active);
    resolv_cli.call(req0, resp0);
}

void notifyService(b0::Node &node, std::string service_name, bool reverse, bool active)
{
    b0::Node::ResolverServiceClient &resolv_cli = node.resolverClient();
    b0::resolver_msgs::Request req0;
    b0::resolver_msgs::NodeServiceRequest &req = *req0.mutable_node_service();
    b0::resolver_msgs::Response resp0;
    req.set_node_name(node.getName());
    req.set_service_name(service_name);
    req.set_reverse(reverse);
    req.set_active(active);
    resolv_cli.call(req0, resp0);
}

} // namespace graph

} // namespace b0


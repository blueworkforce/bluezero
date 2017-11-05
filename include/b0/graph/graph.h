#ifndef GRAPH_H_INCLUDED
#define GRAPH_H_INCLUDED

#include <string>

namespace b0
{

class Node;

namespace graph
{

void notifyTopic(b0::Node &node, std::string topic_name, bool reverse, bool active);

void notifyService(b0::Node &node, std::string service_name, bool reverse, bool active);

} // namespace graph

} // namespace b0

#endif // GRAPH_H_INCLUDED

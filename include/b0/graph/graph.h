#ifndef B0__GRAPH__GRAPH_H__INCLUDED
#define B0__GRAPH__GRAPH_H__INCLUDED

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

#endif // B0__GRAPH__GRAPH_H__INCLUDED

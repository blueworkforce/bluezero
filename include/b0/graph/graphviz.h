#ifndef B0__GRAPH__GRAPHVIZ_H
#define B0__GRAPH__GRAPHVIZ_H

#include <string>

namespace b0
{

namespace resolver_msgs
{

class Graph;

} // namespace resolver_msgs

namespace graph
{

void toGraphviz(const b0::resolver_msgs::Graph &graph, std::string filename, std::string outline_color = "black", std::string topic_color = "blue", std::string service_color = "red");

int renderGraphviz(std::string input, std::string output);

} // namespace graph

} // namespace b0

#endif // B0__GRAPH__GRAPHVIZ_H

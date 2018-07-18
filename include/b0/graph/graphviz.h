#ifndef B0__GRAPH__GRAPHVIZ_H
#define B0__GRAPH__GRAPHVIZ_H

#include <string>

#include <b0/message/graph.h>

namespace b0
{

namespace graph
{

void toGraphviz(const b0::message::Graph &graph, std::string filename, std::string outline_color = "black", std::string topic_color = "blue", std::string service_color = "red");

int renderGraphviz(std::string input, std::string output);

} // namespace graph

} // namespace b0

#endif // B0__GRAPH__GRAPHVIZ_H

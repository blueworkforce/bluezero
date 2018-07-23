#ifndef B0__UTILS__GRAPHVIZ_H
#define B0__UTILS__GRAPHVIZ_H

#include <string>

#include <b0/b0.h>
#include <b0/message/graph/graph.h>

namespace b0
{

namespace graph
{

B0_EXPORT void toGraphviz(const b0::message::graph::Graph &graph, const std::string &filename, const std::string &outline_color = "black", const std::string &topic_color = "blue", const std::string &service_color = "red");

B0_EXPORT int renderGraphviz(const std::string &input, const std::string &output);

} // namespace graph

} // namespace b0

#endif // B0__UTILS__GRAPHVIZ_H

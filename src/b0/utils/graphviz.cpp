#include <b0/utils/graphviz.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#ifdef HAVE_BOOST_PROCESS
#include <boost/process.hpp>
#endif

namespace b0
{

namespace graph
{

static std::string id(const std::string &t, const std::string &name)
{
    boost::format fmt("%s_%s");
    return (fmt % t % boost::replace_all_copy(name, "-", "_")).str();
}

void toGraphviz(const b0::message::graph::Graph &graph, const std::string &filename, const std::string &outline_color, const std::string &topic_color, const std::string &service_color)
{
    std::ofstream f;
    f.open(filename);
    std::set<std::string> nodes;
    std::set<std::string> topics;
    std::set<std::string> services;
    for(auto x : graph.nodes)
    {
        nodes.insert(x);
    }
    for(auto x : graph.node_topic)
    {
        nodes.insert(x.node_name);
        topics.insert(x.other_name);
    }
    for(auto x : graph.node_service)
    {
        nodes.insert(x.node_name);
        services.insert(x.other_name);
    }
    f << "digraph G {" << std::endl;
    f << "    graph [overlap=false, splines=true, bgcolor=\"transparent\"];";
    f << "    node [shape=box, color=" << outline_color << ", fontcolor=" << outline_color << "];";
    for(auto x : nodes) f << id("N", x) << " [label=\"" << x << "\"];";
    f << std::endl;
    f << "    node [shape=ellipse, color=" << topic_color << "];";
    for(auto x : topics) f << id("T", x) << " [label=\"" << x << "\", fontcolor=" << topic_color << "];";
    f << std::endl;
    f << "    node [shape=diamond, color=" << service_color << "];";
    for(auto x : services) f << id("S", x) << " [label=\"" << x << "\", fontcolor=" << service_color << "];";
    f << std::endl;
    f << "    edge [color=" << outline_color << "];" << std::endl;
    for(auto x : graph.node_topic)
        if(x.reversed)
            f << "    " << id("T", x.other_name) << " -> " << id("N", x.node_name) << ";" << std::endl;
        else
            f << "    " << id("N", x.node_name) << " -> " << id("T", x.other_name) << ";" << std::endl;
    for(auto x : graph.node_service)
        if(x.reversed)
            f << "    " << id("S", x.other_name) << " -> " << id("N", x.node_name) << ";" << std::endl;
        else
            f << "    " << id("N", x.node_name) << " -> " << id("S", x.other_name) << ";" << std::endl;
    f << "}" << std::endl;
    f.close();
}

int renderGraphviz(const std::string &input, const std::string &output)
{
#ifdef HAVE_BOOST_PROCESS
    boost::process::child c(boost::process::search_path("neato"), "-Tpng", boost::process::std_out > output, boost::process::std_in < input);
    c.wait();
    return c.exit_code();
#else
    std::cerr << "boost/process.hpp is needed for executing neato" << std::endl;
    return 1;
#endif
}

} // namespace graph

} // namespace b0


#include <cstdlib>
#include <iostream>
#include <boost/process.hpp>
#include <boost/algorithm/string.hpp>
#include "resolver.pb.h"
#include <b0/node.h>
#include <b0/service_client.h>
#include <b0/subscriber.h>

namespace b0
{

namespace graph
{

class Console : public b0::Node
{
public:
    Console()
        : Node("graph_console"),
          cli_(this, "resolver"),
          sub_(this, "graph", &Console::onGraphChanged)
    {
    }

    ~Console()
    {
    }

    void init()
    {
        Node::init();

        b0::resolver_msgs::Request req;
        b0::resolver_msgs::GetGraphRequest &gg = *req.mutable_get_graph();
        b0::resolver_msgs::Response resp;
        log(INFO, "Requesting graph");
        cli_.call(req, resp);
        printOrDisplayGraph("Current graph", resp.get_graph().graph());
    }

    std::string id(std::string t, std::string name)
    {
        static boost::format fmt("%s_%s");
        boost::replace_all(name, "-", "_");
        return (fmt % t % name).str();
    }

    void onGraphChanged(std::string topic, const b0::resolver_msgs::Graph &graph)
    {
        printOrDisplayGraph("Graph has changed", graph);
    }

    void printOrDisplayGraph(std::string message, const b0::resolver_msgs::Graph &graph)
    {
        if(termHasImageCapability())
        {
            log(INFO, message);
            renderAndDisplayGraph(graph);
        }
        else
        {
            log(INFO, "%s: %s", message, graph.DebugString());
        }
    }

    void renderAndDisplayGraph(const b0::resolver_msgs::Graph &graph)
    {
        graphToGraphviz(graph, "graph.gv");

        if(renderGraphviz("graph.gv", "graph.png") == 0)
        {
            displayInlineImage("graph.png");
        }
        else
        {
            std::cerr << "failed to execute neato" << std::endl;
            return;
        }
    }

    void graphToGraphviz(const b0::resolver_msgs::Graph &graph, std::string filename)
    {
        std::ofstream f;
        f.open(filename);
        std::set<std::string> nodes;
        std::set<std::string> topics;
        std::set<std::string> services;
        for(auto x : graph.node_topic())
        {
            nodes.insert(x.a());
            topics.insert(x.b());
        }
        for(auto x : graph.node_service())
        {
            nodes.insert(x.a());
            services.insert(x.b());
        }
        f << "digraph G {" << std::endl;
        f << "    graph [overlap=false, splines=true, bgcolor=\"transparent\"];";
        f << "    node [shape=ellipse, color=white, fontcolor=white];";
        for(auto x : nodes) f << id("N", x) << " [label=\"" << x << "\"];";
        f << std::endl;
        f << "    node [shape=box, color=cyan];";
        for(auto x : topics) f << id("T", x) << " [label=\"" << x << "\"];";
        f << std::endl;
        f << "    node [shape=diamond, color=red];";
        for(auto x : services) f << id("S", x) << " [label=\"" << x << "\"];";
        f << std::endl;
        f << "    edge [color=white];" << std::endl;
        for(auto x : graph.node_topic())
            if(x.reversed())
                f << "    " << id("T", x.b()) << " -> " << id("N", x.a()) << ";" << std::endl;
            else
                f << "    " << id("N", x.a()) << " -> " << id("T", x.b()) << ";" << std::endl;
        for(auto x : graph.node_service())
            if(x.reversed())
                f << "    " << id("S", x.b()) << " -> " << id("N", x.a()) << ";" << std::endl;
            else
                f << "    " << id("N", x.a()) << " -> " << id("S", x.b()) << ";" << std::endl;
        f << "}" << std::endl;
        f.close();
    }

    int renderGraphviz(std::string input, std::string output)
    {
        boost::process::child c(boost::process::search_path("neato"), "-Tpng", boost::process::std_out > output, boost::process::std_in < input);
        c.wait();
        return c.exit_code();
    }

    bool termHasImageCapability()
    {
        char *TERM_PROGRAM = std::getenv("TERM_PROGRAM");
        return TERM_PROGRAM &&
            std::string(TERM_PROGRAM) == "iTerm.app";
    }

    int displayInlineImage(std::string filename)
    {
        boost::process::child c(boost::process::search_path("bash"), "imgcat", "graph.png");
        c.wait();
        return c.exit_code();
    }

protected:
    b0::ServiceClient<b0::resolver_msgs::Request, b0::resolver_msgs::Response> cli_;
    b0::Subscriber<b0::resolver_msgs::Graph> sub_;
};

} // namespace graph

} // namespace b0

int main()
{
    b0::graph::Console console;
    console.init();
    console.spin();
    return 0;
}


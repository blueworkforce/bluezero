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
        cli_.call(req, resp);
        onGraphChanged("graph", resp.get_graph().graph());
    }

    std::string id(std::string t, std::string name)
    {
        static boost::format fmt("%s_%s");
        boost::replace_all(name, "-", "_");
        return (fmt % t % name).str();
    }

    void onGraphChanged(std::string topic, const b0::resolver_msgs::Graph &graph)
    {
        if(strcmp(std::getenv("TERM_PROGRAM"), "iTerm.app") == 0)
        {
            log(INFO, "Graph has changed");

            std::ofstream ss;
            ss.open("graph.gv");

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
            ss << "digraph G {" << std::endl;
            ss << "    graph [overlap=false, splines=true, bgcolor=\"transparent\"];";
            ss << "    node [shape=ellipse, color=white, fontcolor=white];";
            for(auto x : nodes) ss << id("N", x) << " [label=\"" << x << "\"];";
            ss << std::endl;
            ss << "    node [shape=box, color=cyan];";
            for(auto x : topics) ss << id("T", x) << " [label=\"" << x << "\"];";
            ss << std::endl;
            ss << "    node [shape=diamond, color=red];";
            for(auto x : services) ss << id("S", x) << " [label=\"" << x << "\"];";
            ss << std::endl;
            ss << "    edge [color=white];" << std::endl;
            for(auto x : graph.node_topic())
                if(x.reversed())
                    ss << "    " << id("T", x.b()) << " -> " << id("N", x.a()) << ";" << std::endl;
                else
                    ss << "    " << id("N", x.a()) << " -> " << id("T", x.b()) << ";" << std::endl;
            for(auto x : graph.node_service())
                if(x.reversed())
                    ss << "    " << id("S", x.b()) << " -> " << id("N", x.a()) << ";" << std::endl;
                else
                    ss << "    " << id("N", x.a()) << " -> " << id("S", x.b()) << ";" << std::endl;
            ss << "}" << std::endl;

            ss.close();

            boost::process::child c(boost::process::search_path("neato"), "-Tpng",
                    boost::process::std_out > "graph.png",
                    boost::process::std_in < "graph.gv");
            c.wait();
            int result = c.exit_code();
            if(result != 0)
            {
                std::cerr << "failed to execute neato" << std::endl;
                return;
            }
            boost::process::child c1(boost::process::search_path("bash"), "imgcat", "graph.png");
            c1.wait();
        }
        else
        {
            log(INFO, "Graph has changed: %s", graph.DebugString());
        }
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


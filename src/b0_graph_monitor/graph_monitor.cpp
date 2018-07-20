#include <cstdlib>
#include <iostream>
#include <b0/node.h>
#include <b0/subscriber.h>
#include <b0/resolver/client.h>
#include <b0/utils/graphviz.h>
#include <b0/config.h>
#include <b0/utils/env.h>
#ifdef HAVE_BOOST_PROCESS
#include <boost/process.hpp>
#endif

namespace b0
{

namespace graph
{

class Console : public b0::Node
{
public:
    Console()
        : Node("graph_monitor"),
          resolv_cli_(this),
          sub_(this, "graph", &Console::onGraphChanged, this)
    {
    }

    ~Console()
    {
    }

    void init()
    {
        Node::init();

        if(false)
        {
            /*
             * requesting the initial graph is not really needed, as the
             * initialization of this node will cause a change in the graph
             *
             * FIXME: it would be anyway better to have a timeout, and if a graph
             *        is not received in the first 2 seconds, send an explicit request
             */
            log(info, "Requesting graph");
            b0::message::Graph graph;
            resolv_cli_.getGraph(graph);
            printOrDisplayGraph("Current graph", graph);
        }
    }

    void onGraphChanged(const b0::message::Graph &graph)
    {
        printOrDisplayGraph("Graph has changed", graph);
    }

    void printOrDisplayGraph(std::string message, const b0::message::Graph &graph)
    {
        if(termHasImageCapability())
        {
            log(info, message);
            renderAndDisplayGraph(graph);
        }
        else
        {
            log(info, "%s: %d nodes", message, graph.nodes.size());
        }
    }

    void renderAndDisplayGraph(const b0::message::Graph &graph)
    {
        toGraphviz(graph, "graph.gv", "white", "cyan", "red");

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

    bool termHasImageCapability()
    {
        std::string TERM_PROGRAM = b0::env::get("TERM_PROGRAM");
        return TERM_PROGRAM == "iTerm.app";
    }

    int displayInlineImage(std::string filename)
    {
#ifdef HAVE_BOOST_PROCESS
        boost::process::child c(boost::process::search_path("imgcat"), "graph.png");
        c.wait();
        return c.exit_code();
#else
        std::cerr << "boost/process.hpp is needed for inline image display" << std::endl;
        return 1;
#endif
    }

protected:
    b0::resolver::Client resolv_cli_;
    b0::Subscriber sub_;
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


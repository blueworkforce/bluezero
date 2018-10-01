#include <sstream>
#include <vector>
#include <map>
#include <iostream>
#include <b0/node.h>
#include <b0/service_server.h>
#include <b0/publisher.h>
#include "protocol.h"
#ifdef HAVE_BOOST_PROCESS
#include <boost/process.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/dll.hpp>

namespace bp = boost::process;

namespace b0
{

namespace process_manager
{

class ProcessManager : public b0::Node
{
public:
    ProcessManager()
        : Node("process_manager"),
          beacon_pub_(this, "process_manager/beacon")
    {
    }

    ~ProcessManager()
    {
    }

    void handleRequest(const Request &req, Response &rep)
    {
        if(0) {}
#define HANDLER(N) else if(req.N) { rep.N.emplace(); handle_##N(*req.N, *rep.N); }
        HANDLER(start_process)
        HANDLER(stop_process)
        HANDLER(query_process_status)
        HANDLER(list_active_processes)
#undef HANDLER
        else
        {
            log(error, "bad request");
        }
    }

    void handle_start_process(const StartProcessRequest &req, StartProcessResponse &rep)
    {
        if(!canLaunchProgram(req.path))
        {
            rep.success = false;
            rep.error_message = "permission denied";
            return;
        }
        auto c = new bp::child(req.path, req.args);
        children_[c->id()] = c;
        std::cout << "process " << c->id() << " started" << std::endl;
        rep.success = true;
        rep.pid = c->id();
    }

    void handle_stop_process(const StopProcessRequest &req, StopProcessResponse &rep)
    {
        auto it = children_.find(req.pid);
        if(it == children_.end())
        {
            rep.success = false;
            rep.error_message = "no such pid";
            return;
        }
        it->second->terminate();
        rep.success = true;
    }

    void handle_query_process_status(const QueryProcessStatusRequest &req, QueryProcessStatusResponse &rep)
    {
        auto it = children_.find(req.pid);
        if(it == children_.end())
        {
            rep.success = false;
            rep.error_message = "no such pid";
            return;
        }
        auto c = it->second;
        rep.running = c->running();
        rep.exit_code = c->exit_code();
        rep.success = true;
    }

    void handle_list_active_processes(const ListActiveProcessesRequest &req, ListActiveProcessesResponse &rep)
    {
        for(auto &p : children_)
            rep.pids.push_back(p.second->id());
    }

    void announceNode()
    {
        Node::announceNode();
        // hack after announceNode() to get the actual node name and create a
        // unique service name named after node name:
        std::string service_name = getName() + "/control";
        srv_.reset(new b0::ServiceServer(this, service_name, &ProcessManager::handleRequest, this));
    }

    bool canLaunchProgram(std::string p)
    {
        // security measure: only allow certain programs to be launched
        // current implementation: allow programs contained in current directory

        boost::filesystem::path self_path = boost::dll::program_location().parent_path();
        std::string sp = self_path.string();

        // XXX: if this is launched with ./b0_process_manager, sp ends with .
        if(sp[sp.size() - 1] == '.')
            sp = sp.substr(0, sp.size() - 1);

        // filter attempts to bypass this security measure:
        if(p.find("/../") != std::string::npos) return false;

        if(!boost::starts_with(p, sp))
        {
            std::cout << "error: permission denied: '" << p << "' does not start with '" << sp << "'" << std::endl;
            return false;
        }

        return true;
    }

    void cleanup()
    {
        for(auto it = children_.begin(); it != children_.end(); )
        {
            auto c = it->second;
            if(c->running())
            {
                ++it;
            }
            else
            {
                c->wait();
                std::cout << "process " << c->id() << " finished with exit code " << c->exit_code() << std::endl;
                it = children_.erase(it);
                delete c;
            }
        }
    }

    void sendBeacon()
    {
        Beacon beacon;
        beacon.host_name = hostname();
        beacon.node_name = getName();
        beacon.service_name = srv_->getName();
        beacon_pub_.publish(beacon);
    }

    void spinOnce()
    {
        Node::spinOnce();
        cleanup();
        sendBeacon();
    }

protected:
    std::unique_ptr<b0::ServiceServer> srv_;
    b0::Publisher beacon_pub_;
    std::map<pid_t, bp::child*> children_;
};

} // namespace process_manager

} // namespace b0

int main(int argc, char **argv)
{
    b0::init(argc, argv);
    b0::process_manager::ProcessManager node;
    node.init();
    node.spin();
    return 0;
}

#else // HAVE_BOOST_PROCESS

int main()
{
    std::cerr << "boost/process.hpp is needed for process_manager" << std::endl;
    return 1;
}

#endif // HAVE_BOOST_PROCESS

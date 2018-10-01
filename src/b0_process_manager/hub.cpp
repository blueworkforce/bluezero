#include <sstream>
#include <vector>
#include <map>
#include <iostream>
#include <b0/node.h>
#include <b0/service_client.h>
#include <b0/service_server.h>
#include <b0/subscriber.h>
#include "protocol.h"

namespace b0
{

namespace process_manager
{

class HUB : public b0::Node
{
public:
    HUB()
        : Node("process_manager_hub"),
          srv_(this, "process_manager_hub/control", &HUB::handleRequest, this),
          beacon_sub_(this, "process_manager/beacon", &HUB::onBeacon, this)
    {
    }

    ~HUB()
    {
    }

    void onBeacon(const Beacon &beacon)
    {
        //log(trace, "received a beacon from host %s, node %s, srv %s", beacon.host_name, beacon.node_name, beacon.service_name);
        auto it = last_active_.find(beacon.host_name);
        if(it == last_active_.end())
        {
            add(beacon);
        }
        else
        {
            it->second = timeUSec();
        }
    }

    void handleRequest(const Request &req, Response &rep)
    {
        if(!req.host_name)
        {
            log(error, "bad request: missing host_name field");
            return;
        }

        auto it = clients_.find(req.host_name.get());
        if(it == clients_.end())
        {
            log(error, "bad request: unknown host");
            return;
        }

        it->second->call(req, rep);
    }

    void add(const Beacon &beacon)
    {
        last_active_[beacon.host_name] = timeUSec();
        clients_[beacon.host_name].reset(new b0::ServiceClient(this, beacon.service_name, false));
        log(info, "added new entry: %s -> %s", beacon.host_name, beacon.service_name);
        clients_[beacon.host_name]->init();
    }

    void remove(const std::string &host_name)
    {
        auto it = clients_.find(host_name);
        if(it == clients_.end()) return;
        clients_[host_name]->cleanup();
        clients_.erase(it);
        last_active_.erase(host_name);
        log(info, "removed entry: %s", host_name);
    }

    void cleanup()
    {
        std::vector<std::string> removed;
        for(auto it = last_active_.begin(); it != last_active_.end(); ++it)
        {
            int64_t diff = timeUSec() - it->second;
            if(diff > 1000000)
                removed.push_back(it->first);
        }
        for(auto &x : removed)
            remove(x);
    }

    void spinOnce()
    {
        Node::spinOnce();
        cleanup();
    }

protected:
    std::map<std::string, std::unique_ptr<b0::ServiceClient> > clients_;
    std::map<std::string, int64_t > last_active_;
    b0::ServiceServer srv_;
    b0::Subscriber beacon_sub_;
};

} // namespace process_manager

} // namespace b0

int main(int argc, char **argv)
{
    b0::init(argc, argv);
    b0::process_manager::HUB node;
    node.init();
    node.spin();
    return 0;
}


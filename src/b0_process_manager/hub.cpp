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
        auto it = clients_.find(beacon.host_name);
        if(it == clients_.end())
        {
            add(beacon);
            return;
        }
        else
        {
            auto &client = it->second;
            client.last_active_ = timeUSec();
        }
    }

    void handleRequest(const Request &req, Response &rep)
    {
        if(!req.host_name)
        {
            error("bad request: missing host_name field");
            return;
        }

        auto it = clients_.find(req.host_name.get());
        if(it == clients_.end())
        {
            error("bad request: unknown host");
            return;
        }

        auto &client = it->second;
        client.cli_->call(req, rep);
    }

    void add(const Beacon &beacon)
    {
        clients_[beacon.host_name].last_active_ = timeUSec();
        clients_[beacon.host_name].cli_.reset(new b0::ServiceClient(this, beacon.service_name, false));
        info("added new entry: %s -> %s", beacon.host_name, beacon.service_name);
        clients_[beacon.host_name].cli_->init();
    }

    void remove(const std::string &host_name)
    {
        auto it = clients_.find(host_name);
        if(it == clients_.end()) return;
        clients_[host_name].cli_->cleanup();
        clients_.erase(it);
        info("removed entry: %s", host_name);
    }

    void removeInactive()
    {
        std::vector<std::string> removed;
        for(auto it = clients_.begin(); it != clients_.end(); ++it)
        {
            auto &client = it->second;
            int64_t diff = timeUSec() - client.last_active_;
            if(diff > 1000000)
                removed.push_back(it->first);
        }
        for(auto &x : removed)
            remove(x);
    }

    void spinOnce()
    {
        Node::spinOnce();
        removeInactive();
    }

protected:
    struct Client
    {
        std::unique_ptr<b0::ServiceClient> cli_;
        int64_t last_active_;
    };

    std::map<std::string, Client> clients_;
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
    node.cleanup();
    return 0;
}


#include <memory>
#include <string>
#include <vector>
#include <map>
#include <b0/node.h>
#include <b0/publisher.h>
#include "protocol.h"
#ifdef HAVE_BOOST_PROCESS
#include <boost/process.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/regex.hpp>

namespace bp = boost::process;

namespace b0
{

namespace system_monitor
{

class SystemMonitor : public b0::Node
{
public:
    SystemMonitor()
        : Node("system_monitor")
    {
    }

    ~SystemMonitor()
    {
    }

    void announceNode()
    {
        Node::announceNode();
        // hack after announceNode() to get the actual node name and create a
        // unique service name named after node name:
        std::string topic_name = getName();
        pub_.reset(new b0::Publisher(this, topic_name));
    }

    void spinOnce()
    {
        Node::spinOnce();

        Load load_msg;
        load_msg.load_averages = getLoadAverages();
        load_msg.free_memory = getFreeMemory();
        pub_->publish(load_msg);
    }

    std::vector<float> getLoadAverages()
    {
        const int n = 3;
        std::vector<float> avgs;
        bp::ipstream output;
        bp::child c(bp::search_path("uptime"), bp::std_out > output);
        c.wait();
        std::string line;
        if(std::getline(output, line))
        {
            boost::match_results<std::string::const_iterator> matches;
            boost::regex e(".*load averages: ([^ ]+) ([^ ]+) ([^ ]+)");
            if(boost::regex_match(line, matches, e, boost::match_default | boost::match_partial) && matches.size() == n + 1)
            {
                avgs.resize(n);
                for(int i = 0; i < n; i++)
                    avgs[i] = boost::lexical_cast<float>(matches[i + 1]);
            }
            else log(error, "Could not parse output of 'uptime' program");
        }
        else log(error, "Could not read output of 'uptime' program");
        output.pipe().close();
        return avgs;
    }

    int getFreeMemory()
    {
        int r = -1, pgsz = 1;
        bp::ipstream output;
        bp::child c(bp::search_path("vm_stat"), bp::std_out > output);
        c.wait();
        std::string line;
        if(std::getline(output, line))
        {
            boost::match_results<std::string::const_iterator> matches;
            boost::regex e("Mach Virtual Memory Statistics: \\(page size of ([0-9]+) bytes\\)");
            if(boost::regex_match(line, matches, e, boost::match_default | boost::match_partial) && matches.size() == 2)
            {
                pgsz = boost::lexical_cast<int>(matches[1]);

                std::map<std::string, int> stats;
                while(std::getline(output, line))
                {
                    std::vector<std::string> pair;
                    boost::algorithm::split_regex(pair, line, boost::regex(": *"));
                    stats[pair[0]] = int(boost::lexical_cast<float>(pair[1]));
                }
                
                auto it = stats.find("Pages free");
                if(it != stats.end())
                    r = it->second * pgsz;
            }
            else log(error, "Could not parse output of 'vm_stat' program");
        }
        else log(error, "Could not read output of 'vm_stat' program");
        output.pipe().close();
        return r;
    }

protected:
    std::unique_ptr<b0::Publisher> pub_;
};

} // namespace system_monitor

} // namespace b0

int main(int argc, char **argv)
{
    b0::init(argc, argv);
    b0::system_monitor::SystemMonitor node;
    node.init();
    node.spin();
    node.cleanup();
    return 0;
}

#else // HAVE_BOOST_PROCESS

#include <iostream>

int main()
{
    std::cerr << "boost/process.hpp is needed for system_monitor" << std::endl;
    return 1;
}

#endif // HAVE_BOOST_PROCESS

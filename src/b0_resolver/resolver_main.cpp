#include <string>
#include <iostream>

#include <boost/program_options.hpp>

#include <b0/resolver/resolver.h>
#include <b0/logger/interface.h>

using LogLevel = b0::logger::LogInterface::LogLevel;
namespace po = boost::program_options;

int main(int argc, char **argv)
{
    b0::init(argc, argv);

    int64_t minimum_heartbeat_interval = 5000000;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "display help message")
        ("minimum-heartbeat-interval,o", po::value<int64_t>(&minimum_heartbeat_interval), "set minimum heartbeat interval (0 will disable online monitoring)")
    ;
    po::variables_map vm;
    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);
    }
    catch(po::unknown_option &ex)
    {
        std::cerr << ex.what() << std::endl << desc << std::endl;
        return 1;
    }
    po::notify(vm);

    if(vm.count("help"))
    {
        std::cout << desc << std::endl;
        return 0;
    }

    b0::resolver::Resolver node;
    if(vm.count("minimum-heartbeat-interval"))
    {
        node.setMinimumHeartbeatInterval(minimum_heartbeat_interval);
        if(minimum_heartbeat_interval == 0)
            node.log(LogLevel::warn, "Online monitoring is disabled");
    }
    node.init();
    node.spin();
    node.cleanup();

    return 0;
}


#include <string>
#include <iostream>

#include <boost/program_options.hpp>

#include <b0/resolver/resolver.h>
#include <b0/logger/interface.h>

using b0::logger::LogInterface;
namespace po = boost::program_options;

int main(int argc, char **argv)
{
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "display help message")
        ("disable-online-monitoring,o", "disable monitoring of alive nodes")
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
    node.init();
    if(vm.count("disable-online-monitoring"))
    {
        node.setOnlineMonitoring(false);
        node.log(LogInterface::warn, "Online monitoring is disabled");
    }
    node.spin();
    node.cleanup();

    return 0;
}


#include <string>
#include <iostream>

#include <boost/program_options.hpp>

#include <b0/resolver/resolver.h>

namespace po = boost::program_options;

int main(int argc, char **argv)
{
    b0::addOptions()
        ("minimum-heartbeat-interval,o", po::value<int64_t>()->default_value(30000000), "set the minimum heartbeat interval, in microseconds (an interval of 0us will disable online monitoring)")
    ;
    b0::init(argc, argv);

    b0::resolver::Resolver node;
    if(b0::hasOption("minimum-heartbeat-interval"))
    {
        int64_t interval = b0::getOption("minimum-heartbeat-interval").as<int64_t>();
        node.setMinimumHeartbeatInterval(interval);
        if(interval == 0)
            node.warn("Online monitoring is disabled");
    }

    node.init();
    node.spin();
    node.cleanup();

    return 0;
}


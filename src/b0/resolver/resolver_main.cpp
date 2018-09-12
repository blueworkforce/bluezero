#include "resolver.pb.h"
#include <b0/resolver/resolver.h>
#include <b0/logger/interface.h>
#include <string>

using b0::logger::LogInterface;

int main(int argc, char **argv)
{
    b0::resolver::Resolver node;

    node.init();

    for(int i = 1; i < argc; i++)
    {
        std::string v = argv[i];

        if(v == "-o")
        {
            node.setOnlineMonitoring(false);
            node.log(LogInterface::warn, "Online monitoring is disabled");
        }
        else
        {
            node.log(LogInterface::error, "Unrecognized command line option \"%s\"", v);
        }
    }

    node.spin();

    return 0;
}


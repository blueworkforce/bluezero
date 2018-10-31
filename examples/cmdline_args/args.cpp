/*! \example cmdline_args/args.cpp
 * Example of adding custom command-line arguments
 */

//! \cond HIDDEN_SYMBOLS

#include <b0/node.h>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char **argv)
{
    b0::addOptions()
        ("fancy-name,n", po::value<std::string>(), "a string arg")
        ("lucky-number,x", po::value<int>()->default_value(23), "an int arg with default")
        ("file,f", po::value<std::string>(), "file arg")
    ;
    b0::setPositionalOption("file");
    b0::init(argc, argv);

    b0::Node node;

    if(b0::hasOption("fancy-name"))
    {
        std::string n = b0::getOption("fancy-name").as<std::string>();
        // do something with n
    }

    node.init();
    node.spin();
    node.cleanup();

    return 0;
}

//! \endcond

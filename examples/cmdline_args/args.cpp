/*! \example cmdline_args/args.cpp
 * Example of adding custom command-line arguments
 */

//! \cond HIDDEN_SYMBOLS

#include <b0/node.h>

int main(int argc, char **argv)
{
    b0::addOptionString("fancy-name,n", "a string arg");
    b0::addOptionInt("lucky-number,x", "an int arg with default", nullptr, false, 23);
    b0::addOptionString("file,f", "file arg", nullptr, true);
    b0::setPositionalOption("file");
    b0::init(argc, argv);

    b0::Node node;

    if(b0::hasOption("fancy-name"))
    {
        std::string n = b0::getOptionString("fancy-name");
        // do something with n
    }

    node.init();
    node.spin();
    node.cleanup();

    return 0;
}

//! \endcond

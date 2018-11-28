/*! \example cmdline_args/args.cpp
 * Example of adding custom command-line arguments
 */

//! \cond HIDDEN_SYMBOLS

#include <b0/b0.h>
#include <iostream>
using std::cout; using std::endl;

int main(int argc, char **argv)
{
    b0::addOptionString("fancy-name,n", "a string arg");
    b0::addOptionInt("lucky-number,x", "an int arg with default", nullptr, false, 23);
    b0::addOptionStringVector("file,f", "file arg", nullptr, true);
    b0::setPositionalOption("file", -1);
    b0::init(argc, argv);

    for(auto f : b0::getOptionStringVector("file"))
        cout << "file = " << f << endl;

    if(b0::hasOption("lucky-number"))
    {
        int x = b0::getOptionInt("lucky-number");
        cout << "lucky number = " << x << endl;
    }

    return 0;
}

//! \endcond

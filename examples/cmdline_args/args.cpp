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
    b0::addOptionString("file,f", "file arg", nullptr, true);
    b0::setPositionalOption("file");
    b0::init(argc, argv);

    cout << "file = " << b0::getOptionString("file") << endl;
    if(b0::hasOption("lucky-number"))
    {
        int x = b0::getOptionInt("lucky-number");
        cout << "lucky number = " << x << endl;
    }

    return 0;
}

//! \endcond

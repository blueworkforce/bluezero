#include <b0/node.h>
#include <b0/publisher.h>

#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>

/*! \example remapping/const.cpp
 * This node outputs a constant value on its topic
 */

//! \cond HIDDEN_SYMBOLS

namespace po = boost::program_options;

class Const : public b0::Node
{
public:
    Const(int value)
        : b0::Node("const"),
          pub_(this, "out"),
          value_(value)
    {}

    void spinOnce() override
    {
        b0::Node::spinOnce();
        pub_.publish(boost::lexical_cast<std::string>(value_));
    }

private:
    b0::Publisher pub_;
    int value_;
};

int main(int argc, char **argv)
{
    b0::addOptions()
        ("value,v", po::value<int>()->default_value(0), "the value to publish")
    ;
    b0::init(argc, argv);
    Const node(b0::getOption("value").as<int>());
    node.init();
    node.spin();
    node.cleanup();
    return 0;
}

//! \endcond


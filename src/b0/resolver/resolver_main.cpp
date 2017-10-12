#include <b0/resolver/resolver.h>

int main(int argc, char **argv)
{
    b0::resolver::Resolver node;
    node.init();
    node.spin();
    return 0;
}


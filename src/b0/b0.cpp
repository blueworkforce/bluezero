#include <b0/b0.h>

namespace b0
{

static bool initialized = false;

void init(int argc, char **argv)
{
    initialized = true;
}

bool isInitialized()
{
    return initialized;
}

} // namespace b0


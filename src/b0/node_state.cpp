#include <b0/node_state.h>

namespace b0
{

std::string NodeState_str(NodeState s)
{
#define STATE_TO_STRING(s) case NodeState::s: return #s;
    switch(s)
    {
    STATE_TO_STRING(Created)
    STATE_TO_STRING(Ready)
    STATE_TO_STRING(Terminated)
    }
#undef STATE_TO_STRING
}

} // namespace b0


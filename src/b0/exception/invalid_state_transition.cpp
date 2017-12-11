#include <b0/exception/invalid_state_transition.h>

namespace b0
{

namespace exception
{

InvalidStateTransition::InvalidStateTransition(std::string message)
    : Exception(message)
{
}

} // namespace exception

} // namespace b0


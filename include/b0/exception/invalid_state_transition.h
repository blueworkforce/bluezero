#ifndef B0__EXCEPTION__INVALID_STATE_TRANSITION_H__INCLUDED
#define B0__EXCEPTION__INVALID_STATE_TRANSITION_H__INCLUDED

#include <b0/exception/exception.h>

namespace b0
{

namespace exception
{

/*!
 * \brief An exception thrown when an invalid method for the current object state is called
 */
class InvalidStateTransition : public Exception
{
public:
    /*!
     * \brief Construct an InvalidStateTransition exception
     */
    InvalidStateTransition(std::string message = "");
};

} // namespace exception

} // namespace b0

#endif // B0__EXCEPTION__INVALID_STATE_TRANSITION_H__INCLUDED

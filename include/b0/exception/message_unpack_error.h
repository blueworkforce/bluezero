#ifndef B0__EXCEPTION__MESSAGE_UNPACK_ERROR_H__INCLUDED
#define B0__EXCEPTION__MESSAGE_UNPACK_ERROR_H__INCLUDED

#include <b0/exception/exception.h>

namespace b0
{

namespace exception
{

/*!
 * \brief An exception thrown when an invalid method for the current object state is called
 */
class MessageUnpackError : public Exception
{
public:
    /*!
     * \brief Construct an MessageUnpackError exception
     */
    MessageUnpackError(std::string message = "");
};

} // namespace exception

} // namespace b0

#endif // B0__EXCEPTION__MESSAGE_UNPACK_ERROR_H__INCLUDED

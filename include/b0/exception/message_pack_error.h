#ifndef B0__EXCEPTION__MESSAGE_PACK_ERROR_H__INCLUDED
#define B0__EXCEPTION__MESSAGE_PACK_ERROR_H__INCLUDED

#include <b0/exception/exception.h>

namespace b0
{

namespace exception
{

/*!
 * \brief An exception thrown when writing to socket fails
 */
class MessagePackError : public Exception
{
public:
    /*!
     * \brief Construct an MessagePackError exception
     */
    MessagePackError(std::string message = "");
};

class EnvelopeEncodeError : public MessagePackError
{
public:
    EnvelopeEncodeError();
};

class ProtobufSerializeError : public MessagePackError
{
public:
    ProtobufSerializeError();
};

class SocketWriteError : public MessagePackError
{
public:
    SocketWriteError();
};

} // namespace exception

} // namespace b0

#endif // B0__EXCEPTION__MESSAGE_PACK_ERROR_H__INCLUDED

#ifndef B0__EXCEPTION__MESSAGE_UNPACK_ERROR_H__INCLUDED
#define B0__EXCEPTION__MESSAGE_UNPACK_ERROR_H__INCLUDED

#include <b0/exception/exception.h>

namespace b0
{

namespace exception
{

/*!
 * \brief An exception thrown when reading from socket fails
 */
class MessageUnpackError : public Exception
{
public:
    /*!
     * \brief Construct an MessageUnpackError exception
     */
    MessageUnpackError(std::string message = "");
};

class MessageMissingHeaderError : public MessageUnpackError
{
public:
    MessageMissingHeaderError();
};

class MessageTooManyPartsError : public MessageUnpackError
{
public:
    MessageTooManyPartsError();
};

class HeaderMismatch : public MessageUnpackError
{
public:
    HeaderMismatch(std::string header, std::string expected_header);
};

class EnvelopeDecodeError : public MessageUnpackError
{
public:
    EnvelopeDecodeError();
};

class ProtobufParseError : public MessageUnpackError
{
public:
    ProtobufParseError();
};

class SocketReadError : public MessageUnpackError
{
public:
    SocketReadError();
};

class MessageTypeMismatch : public MessageUnpackError
{
public:
    MessageTypeMismatch(std::string type, std::string expected_type);
};

} // namespace exception

} // namespace b0

#endif // B0__EXCEPTION__MESSAGE_UNPACK_ERROR_H__INCLUDED

#ifndef B0__MESSAGE__MESSAGE_ENVELOPE_H__INCLUDED
#define B0__MESSAGE__MESSAGE_ENVELOPE_H__INCLUDED

#include <vector>
#include <string>
#include <utility>
#include <boost/optional.hpp>

#include <b0/b0.h>
#include <b0/message/message.h>

namespace b0
{

namespace message
{

/*!
 * \brief A structure to represent a message part
 *
 * \sa MessageEnvelope
 */
struct MessagePart
{
    //! \brief An optional string indicating the type of the payload
    std::string content_type;

    //! \brief Compression algorithm name, or blank if no compression
    std::string compression_algorithm;

    //! \brief Compression level, or 0 if no compression
    int compression_level;

    //! \brief The payload
    std::string payload;
};

/*!
 * \brief A message envelope used to wrap (optionally: compress) the real message payload(s)
 *
 * A MessageEnvelope consists of a sequence of headers (one per line) followed by
 * a blank line and by a sequence of payloads.
 *
 * The `Part-count` header tells how many MessagePart are contained in the message.
 *
 * Each MessagePart has its own `Content-length` and `Content-type` headers, and can be
 * independently compressed.
 *
 * Example message:
 *
 *     Header: myTopic
 *     Part-count: 2
 *     Content-length-0: 5
 *     Content-type-0: MessageA
 *     Content-length-1: 10
 *     Content-type-1: MessageB
 *     Content-length: 15
 *     
 *     aaaaabbbbbbbbbb
 *
 * The only mandatory fields are `Part-count` and `Content-length-#` which are required
 * to disassemble the individual message parts. The payload size (15) is the sum of the
 * individual (compressed) payloads.
 *
 * If the `Header` header is present (used with Publisher and Subscriber sockets) it must
 * be the first header.
 */
class B0_EXPORT MessageEnvelope final
{
public:
    //! The very first line of the message envelope, used for routing (topics, services)
    std::string header0;

    //! The message parts
    std::vector<MessagePart> parts;

    //! Additional customized headers
    std::map<std::string, std::string> headers;
};

/*!
 * \brief Parse a message envelope from a string
 */
void parse(MessageEnvelope &env, const std::string &s);

/*!
 * \brief Serialize a message envelope to a string
 */
void serialize(const MessageEnvelope &msg, std::string &s);

} // namespace message

} // namespace b0

#endif // B0__MESSAGE__MESSAGE_ENVELOPE_H__INCLUDED

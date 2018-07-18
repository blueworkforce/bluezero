#include <b0/message/message_envelope.h>
#include <b0/exception/message_unpack_error.h>

#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

namespace b0
{

namespace message
{

void MessageEnvelope::parseFromString(const std::string &s)
{
    size_t content_begin = s.find("\n\n");
    if(content_begin == std::string::npos)
        throw exception::EnvelopeDecodeError();
    std::string headers = s.substr(0, content_begin);
    payload = s.substr(content_begin + 2);
    std::vector<std::string> headers_split;
    boost::split(headers_split, headers, boost::is_any_of("\n"));
    for(auto &header_line : headers_split)
    {
        size_t delim_pos = header_line.find(": ");
        if(delim_pos == std::string::npos)
            throw exception::EnvelopeDecodeError();
        std::string key = header_line.substr(0, delim_pos),
            value = header_line.substr(delim_pos + 2);
        if(key == "Content-length")
            content_length = boost::lexical_cast<int>(value);
        else if(key == "Content-type")
            content_type = value;
        else if(key == "Compression-algorithm")
            compression_algorithm = value;
    }
}

void MessageEnvelope::serializeToString(std::string &s) const
{
    std::stringstream ss;
    if(content_length > 0)
        ss << "Content-length: " << content_length << std::endl;
    if(content_type != "")
        ss << "Content-type: " << content_type << std::endl;
    if(compression_algorithm != "")
        ss << "Compression-algorithm: " << compression_algorithm << std::endl;
    ss << std::endl;
    ss << payload;
    s = ss.str();
}

} // namespace message

} // namespace b0


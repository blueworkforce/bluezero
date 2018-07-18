#include <b0/message/message_envelope.h>
#include <b0/exception/message_unpack_error.h>

#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

namespace b0
{

namespace message
{

boost::optional<std::string> MessageEnvelope::getHeader(const std::string &name)
{
    for(auto &x : headers)
    {
        size_t delim_pos = x.second.find(": ");
        if(delim_pos != std::string::npos && x.second.substr(0, delim_pos) == name)
            return x.second.substr(delim_pos + 2);
    }
    return {};
}

void MessageEnvelope::parseFromString(const std::string &s)
{
    size_t content_begin = s.find("\n\n");
    if(content_begin == std::string::npos)
        throw exception::EnvelopeDecodeError();
    std::string message_headers = s.substr(0, content_begin);
    payload = s.substr(content_begin + 2);
    std::vector<std::string> headers_split;
    boost::split(headers_split, message_headers, boost::is_any_of("\n"));
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
        else
            headers.emplace_back(0, header_line);
    }
}

void MessageEnvelope::serializeToString(std::string &s) const
{
    std::vector<std::pair<int, std::string> > all_headers(headers);
    if(content_length > 0)
        all_headers.emplace_back(0, "Content-length: " + boost::lexical_cast<std::string>(content_length));
    if(content_type != "")
        all_headers.emplace_back(100, "Content-type: " + content_type);
    if(compression_algorithm != "")
        all_headers.emplace_back(500, "Compression-algorithm: " + compression_algorithm);
    std::sort(all_headers.begin(), all_headers.end(), [=](std::pair<int, std::string>& a, std::pair<int, std::string>& b)
    {
        return a.first < b.first;
    });
    std::stringstream ss;
    for(auto &x : all_headers)
        ss << x.second << std::endl;
    ss << std::endl;
    ss << payload;
    s = ss.str();
}

} // namespace message

} // namespace b0


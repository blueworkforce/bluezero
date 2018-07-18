#include <b0/message/message_envelope.h>
#include <b0/exception/message_unpack_error.h>

#include <vector>
#include <boost/format.hpp>
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

std::string MessageEnvelope::getHeader(const std::string &name, const std::string &def)
{
    boost::optional<std::string> v = getHeader(name);
    return v ? *v : def;
}

void MessageEnvelope::parseFromString(const std::string &s)
{
    size_t content_begin = s.find("\n\n");
    if(content_begin == std::string::npos)
        throw exception::EnvelopeDecodeError();
    std::string message_headers = s.substr(0, content_begin);
    std::string payload = s.substr(content_begin + 2);
    std::vector<std::string> headers_split;
    boost::split(headers_split, message_headers, boost::is_any_of("\n"));
    int part_count = 0;
    for(auto &header_line : headers_split)
    {
        size_t delim_pos = header_line.find(": ");
        if(delim_pos == std::string::npos)
            throw exception::EnvelopeDecodeError();
        std::string key = header_line.substr(0, delim_pos),
            value = header_line.substr(delim_pos + 2);
        if(key == "Part-count")
            part_count = boost::lexical_cast<int>(value);
        else
            headers.emplace_back(0, header_line);
    }
    parts.resize(part_count);
    int part_start = 0;
    for(int i = 0; i < part_count; i++)
    {
        parts[i].content_length = boost::lexical_cast<int>(getHeader((boost::format("Content-length-%d") % i).str(), "0"));
        parts[i].content_type = getHeader((boost::format("Content-type-%d") % i).str(), "");
        parts[i].compression_algorithm = getHeader((boost::format("Compression-algorithm-%d") % i).str(), "");
        parts[i].uncompressed_content_length = boost::lexical_cast<int>(getHeader((boost::format("Uncompressed-content-length-%d") % i).str(), "0"));
        if(!parts[i].uncompressed_content_length)
            parts[i].uncompressed_content_length = parts[i].content_length;
        parts[i].payload = payload.substr(part_start, parts[i].content_length);
        part_start += parts[i].content_length;
    }
}

void MessageEnvelope::serializeToString(std::string &s) const
{
    std::vector<std::pair<int, std::string> > all_headers(headers);
    all_headers.emplace_back(-10, "Part-count: " + boost::lexical_cast<std::string>(parts.size()));
    for(size_t i = 0; i < parts.size(); i++)
    {
        all_headers.emplace_back(0 + i, (boost::format("Content-length-%d: %d") % i % parts[i].content_length).str());
        if(parts[i].content_type != "")
            all_headers.emplace_back(100 + i, (boost::format("Content-type-%d: %s") % i % parts[i].content_type).str());
        if(parts[i].uncompressed_content_length)
            all_headers.emplace_back(500 + i, (boost::format("Uncompressed-content-length-%d: %d") % i % parts[i].uncompressed_content_length).str());
        if(parts[i].compression_algorithm != "")
            all_headers.emplace_back(500 + i, (boost::format("Compression-algorithm-%d: %s") % i % parts[i].compression_algorithm).str());
    }
    std::sort(all_headers.begin(), all_headers.end());
    std::stringstream ss;
    for(auto &x : all_headers)
        ss << x.second << std::endl;
    ss << std::endl;
    for(auto &part : parts)
        ss << part.payload;
    s = ss.str();
}

} // namespace message

} // namespace b0


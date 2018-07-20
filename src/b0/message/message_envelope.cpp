#include <b0/message/message_envelope.h>
#include <b0/exception/message_unpack_error.h>
#include <b0/compress/compress.h>

#include <vector>
#include <boost/format.hpp>
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
    std::string message_headers = s.substr(0, content_begin);
    std::string payload = s.substr(content_begin + 2);
    std::vector<std::string> headers_split;
    boost::split(headers_split, message_headers, boost::is_any_of("\n"));
    for(auto &header_line : headers_split)
    {
        size_t delim_pos = header_line.find(": ");
        if(delim_pos == std::string::npos)
            throw exception::EnvelopeDecodeError();
        std::string key = header_line.substr(0, delim_pos),
            value = header_line.substr(delim_pos + 2);
        headers[key] = value;
    }
    try
    {
        auto part_count_it = headers.find("Part-count");
        if(part_count_it == headers.end()) throw;
        int part_count = boost::lexical_cast<int>(part_count_it->second);
        headers.erase(part_count_it);
        parts.resize(part_count);
    }
    catch(...) {throw exception::EnvelopeDecodeError();}
    int part_start = 0;
    for(int i = 0; i < parts.size(); i++)
    {
        auto content_type_it = headers.find((boost::format("Content-type-%d") % i).str());
        if(content_type_it != headers.end())
        {
            parts[i].content_type = content_type_it->second;
            headers.erase(content_type_it);
        }

        auto compression_algorithm_it = headers.find((boost::format("Compression-algorithm-%d") % i).str());
        if(compression_algorithm_it != headers.end())
        {
            parts[i].compression_algorithm = compression_algorithm_it->second;
            headers.erase(compression_algorithm_it);
        }

        auto compression_level_it = headers.find((boost::format("Compression-level-%d") % i).str());
        if(compression_level_it != headers.end())
        {
            parts[i].compression_level = boost::lexical_cast<int>(compression_level_it->second);
            headers.erase(compression_level_it);
        }

        auto uncompressed_content_length_it = headers.find((boost::format("Uncompressed-content-length-%d") % i).str());
        int uncompressed_content_length = -1;
        if(uncompressed_content_length_it != headers.end())
        {
            uncompressed_content_length = boost::lexical_cast<int>(uncompressed_content_length_it->second);
            headers.erase(uncompressed_content_length_it);
        }

        auto content_length_it = headers.find((boost::format("Content-length-%d") % i).str());
        int content_length = -1;
        if(content_length_it != headers.end())
        {
            content_length = boost::lexical_cast<int>(content_length_it->second);
            headers.erase(content_length_it);
        }

        if(content_length == -1)
            throw exception::EnvelopeDecodeError();

        parts[i].payload = b0::compress::decompress(parts[i].compression_algorithm, payload.substr(part_start, content_length), uncompressed_content_length);
        part_start += content_length;
    }
}

void MessageEnvelope::serializeToString(std::string &s) const
{
    std::stringstream ss;

    auto header_it = headers.find("Header");
    if(header_it != headers.end())
        ss << "Header: " << header_it->second << std::endl;

    ss << "Part-count: " << parts.size() << std::endl;
    std::vector<std::string> compressed_payloads;
    for(size_t i = 0; i < parts.size(); i++)
    {
        std::string compressed_payload = b0::compress::compress(parts[i].compression_algorithm, parts[i].payload, parts[i].compression_level);
        compressed_payloads.push_back(compressed_payload);

        ss << "Content-length-" << i << ": " << compressed_payload.size() << std::endl;
        if(parts[i].content_type != "")
            ss << "Content-type-" << i << ": " << parts[i].content_type << std::endl;
        if(parts[i].compression_algorithm != "")
        {
            ss << "Compression-algorithm-" << i << ": " << parts[i].compression_algorithm << std::endl;
            ss << "Uncompressed-content-length-" << i << ": " << parts[i].payload.size() << std::endl;
            if(parts[i].compression_level > 0)
                ss << "Compression-level-" << i << ": " << parts[i].compression_level << std::endl;
        }
    }

    for(auto &pair : headers)
        if(pair.first != "Header")
            ss << pair.first << ": " << pair.second << std::endl;

    ss << std::endl;

    for(auto &payload : compressed_payloads)
        ss << payload;

    s = ss.str();
}

} // namespace message

} // namespace b0


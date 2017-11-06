#include <stdexcept>

#include <boost/format.hpp>

#include <b0/config.h>
#include <b0/envelope.h>
#include <b0/utils/compress.h>
#include "envelope.pb.h"

namespace b0
{

std::string wrapEnvelope(std::string payload, std::string compression_algorithm, int compression_level)
{
    b0::core_msgs::MessageEnvelope env;
    env.set_uncompressed_size(payload.size());
    env.set_compression_algorithm(compression_algorithm);
    if(compression_algorithm == "")
    {
        env.set_payload(payload);
    }
#ifdef ZLIB_FOUND
    else if(compression_algorithm == "zlib")
    {
        env.set_payload(b0::utils::compress(payload, compression_level));
    }
#endif
    else throw std::runtime_error((boost::format("unsupported compression algorithm: %s") % compression_algorithm).str());

    std::string raw;
    env.SerializeToString(&raw);
    return raw;
}

std::string unwrapEnvelope(std::string rawData)
{
    b0::core_msgs::MessageEnvelope env;
    if(!env.ParseFromString(rawData))
        return "";

    if(env.compression_algorithm() == "")
    {
        return env.payload();
    }
#ifdef ZLIB_FOUND
    else if(env.compression_algorithm() == "zlib")
    {
        return b0::utils::decompress(env.payload(), env.uncompressed_size());
    }
#endif
    else throw std::runtime_error((boost::format("unsupported compression algorithm: %s") % env.compression_algorithm()).str());
}

} // namespace b0


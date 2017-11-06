#include <stdexcept>

#include <boost/format.hpp>

#include <b0/config.h>
#include <b0/envelope.h>
#include <b0/compress/compress.h>
#include "envelope.pb.h"

namespace b0
{

std::string wrapEnvelope(std::string payload, std::string compression_algorithm, int compression_level)
{
    b0::core_msgs::MessageEnvelope env;
    env.set_uncompressed_size(payload.size());
    env.set_compression_algorithm(compression_algorithm);
    env.set_payload(b0::compress::compress(compression_algorithm, payload, compression_level));
    std::string raw;
    env.SerializeToString(&raw);
    return raw;
}

std::string unwrapEnvelope(std::string rawData)
{
    b0::core_msgs::MessageEnvelope env;
    if(!env.ParseFromString(rawData))
        return "";
    return b0::compress::decompress(env.compression_algorithm(), env.payload(), env.uncompressed_size());
}

} // namespace b0


#ifndef B0__ENVELOPE_H__INCLUDED
#define B0__ENVELOPE_H__INCLUDED

#include <string>

#include <boost/optional.hpp>

namespace b0
{

//! \brief Wrap a payload in a MessageEnvelope
std::string wrapEnvelope(std::string payload, std::string compression_algorithm = "", int compression_level = -1);

//! \brief Unwrap a payload from a serialized MessageEnvelope
std::string unwrapEnvelope(std::string rawData);

} // namespace b0

#endif // B0__ENVELOPE_H__INCLUDED

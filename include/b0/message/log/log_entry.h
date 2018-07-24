#ifndef B0__MESSAGE__LOG__LOG_ENTRY_H__INCLUDED
#define B0__MESSAGE__LOG__LOG_ENTRY_H__INCLUDED

#include <boost/serialization/string.hpp>

#include <b0/b0.h>
#include <b0/message/message.h>
#include <b0/logger/interface.h>

namespace b0
{

namespace message
{

namespace log
{

using LogLevel = ::b0::logger::LogInterface::LogLevel;

/*!
 * \brief A log message sent by node to the 'log' topic
 */
class LogEntry : public Message
{
public:
    //! The name of the node
    std::string node_name;

    //! Severity of the message
    union {
        LogLevel level;
        int level_int;
    };

    //! Content of the message
    std::string message;

    //! Time stamp of the message
    int64_t time_usec;

public:
    std::string type() const override {return "b0.message.log.LogEntry";}
};

} // namespace log

} // namespace message

} // namespace b0

//! \cond HIDDEN_SYMBOLS

namespace spotify
{

namespace json
{

using b0::message::log::LogEntry;

template <>
struct default_codec_t<LogEntry>
{
    static codec::object_t<LogEntry> codec()
    {
        auto codec = codec::object<LogEntry>();
        codec.required("node_name", &LogEntry::node_name);
        codec.required("level", &LogEntry::level_int);
        codec.required("message", &LogEntry::message);
        codec.required("time_usec", &LogEntry::time_usec);
        return codec;
    }
};

} // namespace json

} // namespace spotify

//! \endcond

#endif // B0__MESSAGE__LOG__LOG_ENTRY_H__INCLUDED

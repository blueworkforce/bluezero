#ifndef B0__MESSAGE__LOG_ENTRY_H__INCLUDED
#define B0__MESSAGE__LOG_ENTRY_H__INCLUDED

#include <boost/serialization/string.hpp>

#include <b0/message/message.h>
#include <b0/logger/interface.h>

namespace b0
{

namespace message
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
    LogLevel level;

    //! Content of the message
    std::string message;

    //! Time stamp of the message
    int64_t time_usec;

public:
    std::string type() const override;

private:
    void serialize(serialization::MessageFields &fields) const override;
};

} // namespace message

} // namespace b0

#endif // B0__MESSAGE__LOG_ENTRY_H__INCLUDED

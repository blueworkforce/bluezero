#ifndef B0__LOGGER__CONSOLE_H__INCLUDED
#define B0__LOGGER__CONSOLE_H__INCLUDED

#include <b0/node.h>
#include <b0/protobuf/subscriber.h>
#include "logger.pb.h"

#include <string>
#include <boost/thread.hpp>
#include <boost/format.hpp>

namespace b0
{

namespace logger
{

/*!
 * \brief The logger console node
 */
class Console : public Node
{
public:
    /*!
     * \brief Construct a Console node
     */
    Console();

    /*!
     * \brief Console node destructor
     */
    virtual ~Console();

    /*!
     * Return the name of this node
     */
    std::string getName() const { return "console"; }

    /*!
     * Log message event handler
     */
    virtual void onLogMessage(const b0::logger_msgs::LogEntry &entry);

protected:
    //! Subscriber to "log" topic
    b0::protobuf::Subscriber<b0::logger_msgs::LogEntry> sub_;

private:
    //! A dummy logger to get formatting information from
    b0::logger::LocalLogger dummy_logger_;
};

} // namespace logger

} // namespace b0

#endif // B0__LOGGER__CONSOLE_H__INCLUDED

#ifndef CONSOLE_H_INCLUDED
#define CONSOLE_H_INCLUDED

#include <b0/node.h>
#include <b0/subscriber.h>
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
     * \brief Construct a resolver node
     */
    Console();

    /*!
     * \brief Resolver node destructor
     */
    virtual ~Console();

    std::string getName() const {return "console";}

    virtual void onLogMessage(std::string topic, const b0::logger_msgs::LogEntry &entry);

protected:
    //! Subscriber to "log" topic
    b0::Subscriber<b0::logger_msgs::LogEntry> sub_;

private:
    //! A dummy logger to get formatting information from
    b0::logger::LocalLogger dummy_logger_;
};

} // namespace logger

} // namespace b0

#endif // CONSOLE_H_INCLUDED

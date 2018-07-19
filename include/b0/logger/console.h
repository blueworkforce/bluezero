#ifndef B0__LOGGER__CONSOLE_H__INCLUDED
#define B0__LOGGER__CONSOLE_H__INCLUDED

#include <b0/node.h>
#include <b0/subscriber.h>
#include <b0/message/log_entry.h>

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
    virtual void onLogMessage(const b0::message::LogEntry &msg);

protected:
    //! Subscriber to "log" topic
    b0::Subscriber sub_;

private:
    //! A dummy logger to get formatting information from
    b0::logger::LocalLogger dummy_logger_;
};

} // namespace logger

} // namespace b0

#endif // B0__LOGGER__CONSOLE_H__INCLUDED

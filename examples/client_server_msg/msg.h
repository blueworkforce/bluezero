#ifndef B0__EXAMPLES__CLIENT_SERVER_MSG__MSG_H__INCLUDED
#define B0__EXAMPLES__CLIENT_SERVER_MSG__MSG_H__INCLUDED

#include <b0/message/message.h>

/*!
 * \example client_server_msg/msg.h
 */

//! \cond HIDDEN_SYMBOLS

class AddRequest : public b0::message::Message
{
public:
    int a, b;

public:
    std::string type() const override
    {
        return "AddRequest";
    }

private:
    void serialize(b0::message::serialization::MessageFields &fields) const override
    {
        fields.map("a", &a);
        fields.map("b", &b);
    }
};

class AddReply : public b0::message::Message
{
public:
    int c;

public:
    std::string type() const override
    {
        return "AddReply";
    }

private:
    void serialize(b0::message::serialization::MessageFields &fields) const override
    {
        fields.map("c", &c);
    }
};

//! \endcond

#endif // B0__EXAMPLES__CLIENT_SERVER_MSG__MSG_H__INCLUDED

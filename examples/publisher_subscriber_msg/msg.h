#ifndef B0__EXAMPLES__PUBLISHER_SUBSCRIBER_MSG__MSG_H__INCLUDED
#define B0__EXAMPLES__PUBLISHER_SUBSCRIBER_MSG__MSG_H__INCLUDED

#include <string>
#include <b0/message/message.h>

class MyMsg : public b0::message::Message
{
public:
    std::string greeting;
    int n;

public:
    std::string type() const override
    {
        return "MyMsg";
    }

private:
    void serialize(b0::message::serialization::MessageFields &fields) const override
    {
        fields.map("greeting", &greeting);
        fields.map("n", &n);
    }
};

#endif // B0__EXAMPLES__PUBLISHER_SUBSCRIBER_MSG__MSG_H__INCLUDED

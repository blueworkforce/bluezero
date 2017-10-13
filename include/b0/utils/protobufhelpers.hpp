#ifndef PROTOBUFHELPERS_HPP_INCLUDED
#define PROTOBUFHELPERS_HPP_INCLUDED

#include <b0/utils/zhelpers.hpp>

template<typename T>
static bool
s_recv(zmq::socket_t &socket, T &msg)
{
    std::string msg_str = s_recv(socket);
    return msg.ParseFromString(msg_str);
}

template<typename T>
static bool
s_send(zmq::socket_t &socket, const T &msg)
{
    std::string msg_str;
    if(msg.SerializeToString(&msg_str))
    {
        s_send(socket, msg_str);
        return true;
    }
    else return false;
}

#endif // PROTOBUFHELPERS_HPP_INCLUDED


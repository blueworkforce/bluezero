#ifndef B0__UTILS__PROTOBUFHELPERS_H__INCLUDED
#define B0__UTILS__PROTOBUFHELPERS_H__INCLUDED

#include <b0/utils/zhelpers.h>

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

template<typename T>
static bool
s_sendmore(zmq::socket_t &socket, const T &msg)
{
    std::string msg_str;
    if(msg.SerializeToString(&msg_str))
    {
        s_sendmore(socket, msg_str);
        return true;
    }
    else return false;
}

#endif // B0__UTILS__PROTOBUFHELPERS_H__INCLUDED


#include <b0/b0.h>
#include <b0/node.h>
#include <b0/publisher.h>
#include <b0/subscriber.h>
#include <b0/service_client.h>
#include <b0/service_server.h>
extern "C"
{
#include <b0/bindings/c.h>
}

#define B0_EXCEPTIONS_CATCH(name) catch(b0::exception::Exception &name)
#define B0_SUCCESS 1
#define B0_FAILURE 0
#define B0_EXCEPTIONS_WRAPPER_BEGIN()      \
    try                                    \
    {                                      \
        ((void)0)
#define B0_EXCEPTIONS_WRAPPER_BEGIN_RET()  \
    try                                    \
    {                                      \
        ((void)0)
#define B0_EXCEPTIONS_WRAPPER_END()        \
        return B0_SUCCESS;                 \
    }                                      \
    B0_EXCEPTIONS_CATCH(ex)                \
    {                                      \
        return B0_FAILURE;                 \
    }
#define B0_EXCEPTIONS_WRAPPER_END_RET(ret) \
    }                                      \
    B0_EXCEPTIONS_CATCH(ex)                \
    {                                      \
        return ret;                        \
    }

static void b0_socket_set_option(b0::Socket *psock, int option, int value)
{
    switch(option)
    {
    case B0_SOCK_OPT_LINGERPERIOD:
        psock->setLingerPeriod(value);
        break;
    case B0_SOCK_OPT_BACKLOG:
        psock->setBacklog(value);
        break;
    case B0_SOCK_OPT_READTIMEOUT:
        psock->setReadTimeout(value);
        break;
    case B0_SOCK_OPT_WRITETIMEOUT:
        psock->setWriteTimeout(value);
        break;
    case B0_SOCK_OPT_IMMEDIATE:
        psock->setImmediate(value);
        break;
    case B0_SOCK_OPT_CONFLATE:
        psock->setConflate(value);
        break;
    case B0_SOCK_OPT_READHWM:
        psock->setReadHWM(value);
        break;
    case B0_SOCK_OPT_WRITEHWM:
        psock->setWriteHWM(value);
        break;
    }
}

static b0::logger::Level log_level_from_int(int level)
{
    switch(level)
    {
    case B0_FATAL:
        return b0::logger::Level::fatal;
    case B0_ERROR:
        return b0::logger::Level::error;
    case B0_WARN:
        return b0::logger::Level::warn;
    case B0_INFO:
        return b0::logger::Level::info;
    case B0_DEBUG:
        return b0::logger::Level::debug;
    case B0_TRACE:
        return b0::logger::Level::trace;
    default:
        return b0::logger::Level::info;
    }
}

extern "C"
{
int b0_init(int *argc, char **argv)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    b0::init(*argc, argv);
    B0_EXCEPTIONS_WRAPPER_END();
}

void * b0_buffer_new(size_t size)
{
    return reinterpret_cast<void*>(new char[size]);
}

void b0_buffer_delete(void *buffer)
{
    delete reinterpret_cast<char*>(buffer);
}

b0_node * b0_node_new(const char *name)
{
    return reinterpret_cast<b0_node*>(new b0::Node(name));
}

void b0_node_delete(b0_node *node)
{
    delete reinterpret_cast<b0::Node*>(node);
}

int b0_node_init(b0_node *node)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    reinterpret_cast<b0::Node*>(node)->init();
    B0_EXCEPTIONS_WRAPPER_END();
}

int b0_node_shutdown(b0_node *node)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    reinterpret_cast<b0::Node*>(node)->shutdown();
    B0_EXCEPTIONS_WRAPPER_END();
}

int b0_node_shutdown_requested(b0_node *node)
{
    return reinterpret_cast<b0::Node*>(node)->shutdownRequested();
}

int b0_node_spin_once(b0_node *node)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    reinterpret_cast<b0::Node*>(node)->spinOnce();
    B0_EXCEPTIONS_WRAPPER_END();
}

int b0_node_spin(b0_node *node)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    reinterpret_cast<b0::Node*>(node)->spin();
    B0_EXCEPTIONS_WRAPPER_END();
}

int b0_node_cleanup(b0_node *node)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    reinterpret_cast<b0::Node*>(node)->cleanup();
    B0_EXCEPTIONS_WRAPPER_END();
}

const char * b0_node_get_name(b0_node *node)
{
    return reinterpret_cast<b0::Node*>(node)->getName().c_str();
}

int b0_node_get_state(b0_node *node)
{
    return reinterpret_cast<b0::Node*>(node)->getState();
}

void * b0_node_get_context(b0_node *node)
{
    return reinterpret_cast<b0::Node*>(node)->getContext();
}

int64_t b0_node_hardware_time_usec(b0_node *node)
{
    return reinterpret_cast<b0::Node*>(node)->hardwareTimeUSec();
}

int64_t b0_node_time_usec(b0_node *node)
{
    return reinterpret_cast<b0::Node*>(node)->timeUSec();
}

void b0_node_sleep_usec(b0_node *node, int64_t usec)
{
    reinterpret_cast<b0::Node*>(node)->sleepUSec(usec);
}

int b0_node_log(b0_node *node, int level, const char *message)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    reinterpret_cast<b0::Node*>(node)->log(log_level_from_int(level), message);
    B0_EXCEPTIONS_WRAPPER_END();
}

b0_publisher * b0_publisher_new_ex(b0_node *node, const char *topic_name, int managed, int notify_graph)
{
    return reinterpret_cast<b0_publisher*>(new b0::Publisher(reinterpret_cast<b0::Node*>(node), topic_name, managed, notify_graph));
}

b0_publisher * b0_publisher_new(b0_node *node, const char *topic_name)
{
    return b0_publisher_new_ex(node, topic_name, true, true);
}

void b0_publisher_delete(b0_publisher *pub)
{
    delete reinterpret_cast<b0::Publisher*>(pub);
}

int b0_publisher_init(b0_publisher *pub)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    reinterpret_cast<b0::Publisher*>(pub)->init();
    B0_EXCEPTIONS_WRAPPER_END();
}

int b0_publisher_cleanup(b0_publisher *pub)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    reinterpret_cast<b0::Publisher*>(pub)->cleanup();
    B0_EXCEPTIONS_WRAPPER_END();
}

int b0_publisher_spin_once(b0_publisher *pub)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    reinterpret_cast<b0::Publisher*>(pub)->spinOnce();
    B0_EXCEPTIONS_WRAPPER_END();
}

const char * b0_publisher_get_topic_name(b0_publisher *pub)
{
    return reinterpret_cast<b0::Publisher*>(pub)->getTopicName().c_str();
}

int b0_publisher_publish(b0_publisher *pub, const void *data, size_t size)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    std::string msg((const char *)data, size);
    reinterpret_cast<b0::Publisher*>(pub)->publish(msg);
    B0_EXCEPTIONS_WRAPPER_END();
}

int b0_publisher_log(b0_publisher *pub, int level, const char *message)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    reinterpret_cast<b0::Publisher*>(pub)->log(log_level_from_int(level), message);
    B0_EXCEPTIONS_WRAPPER_END();
}

int b0_publisher_set_option(b0_publisher *pub, int option, int value)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    b0_socket_set_option(reinterpret_cast<b0::Publisher*>(pub), option, value);
    B0_EXCEPTIONS_WRAPPER_END();
}

void b0_subscriber_callback_wrapper(const std::string &msg, void (*callback)(const void *, size_t))
{
    (*callback)(msg.data(), msg.size());
}

b0_subscriber * b0_subscriber_new_ex(b0_node *node, const char *topic_name, void (*callback)(const void *, size_t), int managed, int notify_graph)
{
    boost::function<void(const std::string&)> cb_arg;
    if(callback)
        cb_arg = boost::bind(b0_subscriber_callback_wrapper, _1, callback);
    return reinterpret_cast<b0_subscriber*>(new b0::Subscriber(reinterpret_cast<b0::Node*>(node), topic_name, cb_arg, managed, notify_graph));
}

b0_subscriber * b0_subscriber_new(b0_node *node, const char *topic_name, void (*callback)(const void *, size_t))
{
    return b0_subscriber_new_ex(node, topic_name, callback, true, true);
}

void b0_subscriber_delete(b0_subscriber *sub)
{
    delete reinterpret_cast<b0::Subscriber*>(sub);
}

int b0_subscriber_init(b0_subscriber *sub)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    reinterpret_cast<b0::Subscriber*>(sub)->init();
    B0_EXCEPTIONS_WRAPPER_END();
}

int b0_subscriber_cleanup(b0_subscriber *sub)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    reinterpret_cast<b0::Subscriber*>(sub)->cleanup();
    B0_EXCEPTIONS_WRAPPER_END();
}

int b0_subscriber_spin_once(b0_subscriber *sub)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    reinterpret_cast<b0::Subscriber*>(sub)->spinOnce();
    B0_EXCEPTIONS_WRAPPER_END();
}

const char * b0_subscriber_get_topic_name(b0_subscriber *sub)
{
    return reinterpret_cast<b0::Subscriber*>(sub)->getTopicName().c_str();
}

int b0_subscriber_log(b0_subscriber *sub, int level, const char *message)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    reinterpret_cast<b0::Subscriber*>(sub)->log(log_level_from_int(level), message);
    B0_EXCEPTIONS_WRAPPER_END();
}

int b0_subscriber_poll(b0_subscriber *sub, long timeout)
{
    return reinterpret_cast<b0::Subscriber*>(sub)->poll(timeout);
}

void * b0_subscriber_read(b0_subscriber *sub, size_t *size)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN_RET();
    std::string msg;
    reinterpret_cast<b0::Subscriber*>(sub)->readRaw(msg);
    void *ret = b0_buffer_new(msg.size());
    if(!ret) return NULL;
    memcpy(ret, msg.data(), msg.size());
    if(size) *size = msg.size();
    return ret;
    B0_EXCEPTIONS_WRAPPER_END_RET(NULL);
}

int b0_subscriber_set_option(b0_subscriber *sub, int option, int value)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    b0_socket_set_option(reinterpret_cast<b0::Subscriber*>(sub), option, value);
    B0_EXCEPTIONS_WRAPPER_END();
}

b0_service_client * b0_service_client_new_ex(b0_node *node, const char *service_name, int managed, int notify_graph)
{
    return reinterpret_cast<b0_service_client*>(new b0::ServiceClient(reinterpret_cast<b0::Node*>(node), service_name, managed, notify_graph));
}

b0_service_client * b0_service_client_new(b0_node *node, const char *service_name)
{
    return b0_service_client_new_ex(node, service_name, true, true);
}

void b0_service_client_delete(b0_service_client *cli)
{
    delete reinterpret_cast<b0::ServiceClient*>(cli);
}

int b0_service_client_init(b0_service_client *cli)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    reinterpret_cast<b0::ServiceClient*>(cli)->init();
    B0_EXCEPTIONS_WRAPPER_END();
}

int b0_service_client_cleanup(b0_service_client *cli)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    reinterpret_cast<b0::ServiceClient*>(cli)->cleanup();
    B0_EXCEPTIONS_WRAPPER_END();
}

int b0_service_client_spin_once(b0_service_client *cli)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    reinterpret_cast<b0::ServiceClient*>(cli)->spinOnce();
    B0_EXCEPTIONS_WRAPPER_END();
}

const char * b0_service_client_get_service_name(b0_service_client *cli)
{
    return reinterpret_cast<b0::ServiceClient*>(cli)->getServiceName().c_str();
}

void * b0_service_client_call(b0_service_client *cli, const void *data, size_t size, size_t *out_size)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN_RET();
    std::string req((const char *)data, size);
    std::string rep;
    reinterpret_cast<b0::ServiceClient*>(cli)->call(req, rep);
    void *ret = b0_buffer_new(rep.size());
    if(!ret) return NULL;
    memcpy(ret, rep.data(), rep.size());
    *out_size = rep.size();
    return ret;
    B0_EXCEPTIONS_WRAPPER_END_RET(NULL);
}

int b0_service_client_log(b0_service_client *cli, int level, const char *message)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    reinterpret_cast<b0::ServiceClient*>(cli)->log(log_level_from_int(level), message);
    B0_EXCEPTIONS_WRAPPER_END();
}

int b0_service_client_set_option(b0_service_client *cli, int option, int value)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    b0_socket_set_option(reinterpret_cast<b0::ServiceClient*>(cli), option, value);
    B0_EXCEPTIONS_WRAPPER_END();
}

void b0_service_server_callback_wrapper(const std::string &req, std::string &rep, void * (*callback)(const void *, size_t, size_t *))
{
    size_t s;
    void *d = (*callback)(req.data(), req.size(), &s);
    rep = std::string((char*)d, s);
    b0_buffer_delete(d);
}

b0_service_server * b0_service_server_new_ex(b0_node *node, const char *service_name, void * (*callback)(const void *, size_t, size_t *), int managed, int notify_graph)
{
    boost::function<void(const std::string&, std::string&)> cb_arg;
    if(callback)
        cb_arg = boost::bind(b0_service_server_callback_wrapper, _1, _2, callback);
    return reinterpret_cast<b0_service_server*>(new b0::ServiceServer(reinterpret_cast<b0::Node*>(node), service_name, cb_arg, managed, notify_graph));
}

b0_service_server * b0_service_server_new(b0_node *node, const char *service_name, void * (*callback)(const void *, size_t, size_t *))
{
    return b0_service_server_new_ex(node, service_name, callback, true, true);
}

void b0_service_server_delete(b0_service_server *srv)
{
    delete reinterpret_cast<b0::ServiceServer*>(srv);
}

int b0_service_server_init(b0_service_server *srv)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    reinterpret_cast<b0::ServiceServer*>(srv)->init();
    B0_EXCEPTIONS_WRAPPER_END();
}

int b0_service_server_cleanup(b0_service_server *srv)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    reinterpret_cast<b0::ServiceServer*>(srv)->cleanup();
    B0_EXCEPTIONS_WRAPPER_END();
}

int b0_service_server_spin_once(b0_service_server *srv)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    reinterpret_cast<b0::ServiceServer*>(srv)->spinOnce();
    B0_EXCEPTIONS_WRAPPER_END();
}

const char * b0_service_server_get_service_name(b0_service_server *srv)
{
    return reinterpret_cast<b0::ServiceServer*>(srv)->getServiceName().c_str();
}

int b0_service_server_log(b0_service_server *srv, int level, const char *message)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    reinterpret_cast<b0::ServiceServer*>(srv)->log(log_level_from_int(level), message);
    B0_EXCEPTIONS_WRAPPER_END();
}

int b0_service_server_poll(b0_service_server *srv, long timeout)
{
    return reinterpret_cast<b0::ServiceServer*>(srv)->poll(timeout);
}

void * b0_service_server_read(b0_service_server *srv, size_t *size)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN_RET();
    std::string msg;
    reinterpret_cast<b0::ServiceServer*>(srv)->readRaw(msg);
    void *ret = b0_buffer_new(msg.size());
    if(!ret) return NULL;
    memcpy(ret, msg.data(), msg.size());
    if(size) *size = msg.size();
    return ret;
    B0_EXCEPTIONS_WRAPPER_END_RET(NULL);
}

int b0_service_server_write(b0_service_server *srv, const void *msg, size_t size)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    reinterpret_cast<b0::ServiceServer*>(srv)->writeRaw(std::string(reinterpret_cast<const char *>(msg), size));
    B0_EXCEPTIONS_WRAPPER_END();
}

int b0_service_server_set_option(b0_service_server *srv, int option, int value)
{
    B0_EXCEPTIONS_WRAPPER_BEGIN();
    b0_socket_set_option(reinterpret_cast<b0::ServiceServer*>(srv), option, value);
    B0_EXCEPTIONS_WRAPPER_END();
}

}

#ifndef B0_C_H_INCLUDED
#define B0_C_H_INCLUDED

#include <inttypes.h>

#define B0_FATAL 600
#define B0_ERROR 500
#define B0_WARN  400
#define B0_INFO  300
#define B0_DEBUG 200
#define B0_TRACE 100

struct b0_node;
typedef struct b0_node b0_node;

struct b0_publisher;
typedef struct b0_publisher b0_publisher;

struct b0_subscriber;
typedef struct b0_subscriber b0_subscriber;

struct b0_service_client;
typedef struct b0_service_client b0_service_client;

struct b0_service_server;
typedef struct b0_service_server b0_service_server;

void * b0_buffer_new(size_t size);
void b0_buffer_delete(void *buffer);

b0_node * b0_node_new(const char *name);
void b0_node_delete(b0_node *node);
void b0_node_init(b0_node *node);
void b0_node_shutdown(b0_node *node);
int b0_node_shutdown_requested(b0_node *node);
void b0_node_spin_once(b0_node *node);
void b0_node_spin(b0_node *node);
void b0_node_cleanup(b0_node *node);
const char * b0_node_get_name(b0_node *node);
int b0_node_get_state(b0_node *node);
void * b0_node_get_context(b0_node *node);
int64_t b0_node_hardware_time_usec(b0_node *node);
int64_t b0_node_time_usec(b0_node *node);
void b0_node_log(b0_node *node, int level, const char *message);

b0_publisher * b0_publisher_new_ex(b0_node *node, const char *topic_name, int managed, int notify_graph);
b0_publisher * b0_publisher_new(b0_node *node, const char *topic_name);
void b0_publisher_delete(b0_publisher *pub);
void b0_publisher_init(b0_publisher *pub);
void b0_publisher_cleanup(b0_publisher *pub);
void b0_publisher_spin_once(b0_publisher *pub);
const char * b0_publisher_get_topic_name(b0_publisher *pub);
void b0_publisher_publish(b0_publisher *pub, const void *data, size_t size);
void b0_publisher_log(b0_publisher *pub, int level, const char *message);

b0_subscriber * b0_subscriber_new_ex(b0_node *node, const char *topic_name, void (*callback)(const void *, size_t), int managed, int notify_graph);
b0_subscriber * b0_subscriber_new(b0_node *node, const char *topic_name, void (*callback)(const void *, size_t));
void b0_subscriber_delete(b0_subscriber *sub);
void b0_subscriber_init(b0_subscriber *sub);
void b0_subscriber_cleanup(b0_subscriber *sub);
void b0_subscriber_spin_once(b0_subscriber *sub);
const char * b0_subscriber_get_topic_name(b0_subscriber *sub);
void b0_subscriber_log(b0_subscriber *sub, int level, const char *message);
int b0_subscriber_poll(b0_subscriber *sub, long timeout);
void * b0_subscriber_read(b0_subscriber *sub, size_t *size);

b0_service_client * b0_service_client_new_ex(b0_node *node, const char *service_name, int managed, int notify_graph);
b0_service_client * b0_service_client_new(b0_node *node, const char *service_name);
void b0_service_client_delete(b0_service_client *cli);
void b0_service_client_init(b0_service_client *cli);
void b0_service_client_cleanup(b0_service_client *cli);
void b0_service_client_spin_once(b0_service_client *cli);
const char * b0_service_client_get_service_name(b0_service_client *cli);
void * b0_service_client_call(b0_service_client *cli, const void *data, size_t size, size_t *out_size);
void b0_service_client_log(b0_service_client *cli, int level, const char *message);

b0_service_server * b0_service_server_new_ex(b0_node *node, const char *service_name, void * (*callback)(const void *, size_t, size_t *), int managed, int notify_graph);
b0_service_server * b0_service_server_new(b0_node *node, const char *service_name, void * (*callback)(const void *, size_t, size_t *));
void b0_service_server_delete(b0_service_server *srv);
void b0_service_server_init(b0_service_server *srv);
void b0_service_server_cleanup(b0_service_server *srv);
void b0_service_server_spin_once(b0_service_server *srv);
const char * b0_service_server_get_service_name(b0_service_server *srv);
void b0_service_server_log(b0_service_server *srv, int level, const char *message);
int b0_service_server_poll(b0_service_server *srv, long timeout);
void * b0_service_server_read(b0_service_server *srv, size_t *size);
void b0_service_server_write(b0_service_server *srv, const void *msg, size_t size);

#endif // B0_C_H_INCLUDED

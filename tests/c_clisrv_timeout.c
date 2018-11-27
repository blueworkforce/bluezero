#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
//#include <sys/wait.h>
#include <b0/bindings/c.h>

pid_t resolver_pid = -1;

void start_resolver()
{
    resolver_pid = fork();

    if(resolver_pid == -1)
    {
        fprintf(stderr, "error: fork failed\n");
        exit(1);
    }
    else if(resolver_pid > 0)
    {
        //printf("waiting for child...\n");
        //int status;
        //waitpid(pid, &status, 0);
        //printf("child finished\n");
        sleep(2);
    }
    else
    {
        char *argv[2], *envp[2];
        argv[0] = strdup("b0_resolver");
        argv[1] = NULL;
        envp[0] = strdup("B0_HOST_ID=localhost");
        envp[1] = NULL;
        execve("b0_resolver", argv, envp);
        exit(1); // exec() never returns
    }

}

void kill_resolver(int sig)
{
    if(resolver_pid > 0)
        kill(resolver_pid, SIGKILL);
}

void * server_callback(const void *req, size_t sz, size_t *out_sz)
{
    printf("Received: %s\n", (const char*)req);

    int wait = 10;
    printf("Waiting %d seconds...\n", wait);
    sleep(wait);

    const char *repmsg = "hi";
    printf("Sending: %s\n", repmsg);

    *out_sz = strlen(repmsg);
    void *rep = b0_buffer_new(*out_sz);
    memcpy(rep, repmsg, *out_sz);
    return rep;
}

int main(int argc, char **argv)
{
    signal(SIGTERM, (void (*)(int))kill_resolver);
    signal(SIGINT, (void (*)(int))kill_resolver);
    signal(SIGABRT, (void (*)(int))kill_resolver);

    start_resolver();

    b0_init(&argc, argv);

    const char *service_name = "test_service";

    b0_node *server_node = b0_node_new("server");
    b0_service_server *srv = b0_service_server_new(server_node, service_name, &server_callback);
    b0_node_init(server_node);

    b0_node *client_node = b0_node_new("client");
    b0_service_client *cli = b0_service_client_new(client_node, service_name);
    b0_service_client_set_option(cli, B0_SOCK_OPT_READTIMEOUT, 2000);
    b0_node_init(client_node);

    while(!b0_node_shutdown_requested(server_node) &&
          !b0_node_shutdown_requested(client_node))
    {
        b0_node_spin_once(server_node);
        b0_node_spin_once(client_node);
        b0_node_sleep_usec(server_node, 10000);

        const char *req = "hello";
        printf("Sending: %s\n", req);
        char *rep;
        size_t rep_sz;
        // call will block, server will never process request:
        rep = b0_service_client_call(cli, req, strlen(req) + 1, &rep_sz);
        if(rep)
        {
            printf("Received: %s\n", rep);
            b0_buffer_delete(rep);
        }
        else
        {
            printf("Service call failed (timeout?)\n");
        }
        break;
    }

    b0_node_cleanup(server_node);
    b0_node_cleanup(client_node);

    b0_service_server_delete(srv);
    b0_service_client_delete(cli);
    b0_node_delete(server_node);
    b0_node_delete(client_node);

    kill_resolver(SIGTERM);

    int status;
    waitpid(resolver_pid, &status, 0);

    return 0;
}
